#include <alsa/asoundlib.h>

#include <r.h>

#include <linux/fb.h>
#include <linux/ioctl.h>
#include <stropts.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


struct mark {
    const char* what;
    double factor;
    const char* unit;

    struct timespec t;
    size_t count;
    size_t check_period;
};

static void mark_set(struct mark* const m)
{
    int r = clock_gettime(CLOCK_MONOTONIC_RAW, &m->t);
    CHECK(r, "clock_gettime(CLOCK_MONOTONIC_RAW, ..)");

    m->count = 0;
}

static void mark_init(struct mark* const m,
                      const char* const what,
                      const double factor,
                      const char* const unit,
                      const size_t check_period)
{
    m->what = what;
    m->factor = factor;
    m->unit = unit;
    m->check_period = check_period;
    mark_set(m);
}

static void mark_tick(struct mark* const m)
{
    m->count += 1;

    if(m->count == m->check_period) {
        const struct timespec old = m->t;
        mark_set(m);
        const time_t secs = m->t.tv_sec - old.tv_sec;
        const time_t nanos = m->t.tv_nsec - old.tv_nsec;
        const double freq =
            m->check_period/(secs + ((double)nanos)/1000000000) * m->factor;
        info("%s: %f%s", m->what, freq, m->unit);
    }
}

struct {
    snd_seq_t* seq;
    int seq_input_port;
    size_t sync;
    size_t frame;

    struct fb_fix_screeninfo fi;
    struct fb_var_screeninfo vi;
    struct fb_var_screeninfo orig_vi;
    int fbfd;
    char* fb;
    size_t page_size;
    uint_fast8_t cur_page;

    struct mark frames;
    struct mark syncs;
} ctx;

static void initialize_context(void)
{
    memset(&ctx, 0, sizeof(ctx));
    ctx.fbfd = -1;
}

static void clear(uint_fast8_t page)
{
    memset(ctx.fb + ctx.page_size * page, 0, ctx.page_size);
}

typedef uint_fast32_t color_t;

static inline void put_pixel(const size_t x, const size_t y, const color_t c)
{
    const size_t off =
        (x * 3) + y * ctx.fi.line_length + ctx.cur_page * ctx.page_size;
    memcpy(ctx.fb + off, &c, 3);
}

static void draw_rect(const size_t x, const size_t y,
                      const size_t h, const size_t w,
                      const color_t c)
{
    for(size_t i = 0; i < w; i++) {
        for(size_t j = 0; j < h; j++) {
            put_pixel(x + i, y + j, c);
        }
    }
}

static void midi_initialize(const int src_client, const int src_port)
{
    memset(&ctx, 0, sizeof(ctx));

    int r = snd_seq_open(&ctx.seq, "default", SND_SEQ_OPEN_INPUT, 0);
    CHECK_ALSA(r, "unable to open ALSA sequencer");

    r = snd_seq_set_client_name(ctx.seq, "demo");
    CHECK_ALSA(r, "unable to set client name");

    r = snd_seq_create_simple_port(
        ctx.seq, "input port",
        SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
        SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    CHECK_ALSA(r, "unable to create input port");
    ctx.seq_input_port = r;

    r = snd_seq_connect_from(ctx.seq, ctx.seq_input_port, src_client, src_port);
    CHECK_ALSA(r, "unable to connect to source port");

    info("%d:%d -> %d:%d",
         src_client, src_port,
         snd_seq_client_id(ctx.seq), ctx.seq_input_port);
}

void render(void)
{
    clear(ctx.cur_page);
    draw_rect(0   + 5*ctx.frame, 0  , 300, 300, 0xff0000);
    draw_rect(300 + 5*ctx.frame, 300, 300, 300, 0x00ff00);
    draw_rect(600 + 5*ctx.frame, 600, 300, 300, 0x0000ff);

    ctx.vi.yoffset = ctx.cur_page * ctx.vi.yres;
    ctx.vi.activate = FB_ACTIVATE_VBL;
    int r = ioctl(ctx.fbfd, FBIOPAN_DISPLAY, &ctx.vi);
    CHECK(r, "ioctl(FBIOPAN_DISPLAY)");
    r = ioctl(ctx.fbfd, FBIO_WAITFORVSYNC, NULL);
    CHECK(r, "ioctl(FBIO_WAITFORVSYNC)");

    ctx.cur_page = (ctx.cur_page + 1) % 2;
    ctx.frame += 1;
}

void midi_next(void)
{
    snd_seq_event_t* ev;
    int r = snd_seq_event_input(ctx.seq, &ev);
    CHECK_ALSA(r, "unable to retrieve input command");

    switch(ev->type) {
    case SND_SEQ_EVENT_CLOCK:
        mark_tick(&ctx.syncs);

        if(ctx.sync % 24 == 0) {
            /*render();*/
            mark_tick(&ctx.frames);
        }
        ctx.sync += 1;
        break;

    case SND_SEQ_EVENT_NOTEON:
    default: break;
    }

    r = snd_seq_free_event(ev);
    CHECK_ALSA(r, "unable to free event");
}

void open_framebuffer(const char* const fbfn)
{
    ctx.fbfd = open(fbfn, O_RDWR); CHECK(ctx.fbfd, "open(%s, O_RDWR)", fbfn);

    int r = ioctl(ctx.fbfd, FBIOGET_VSCREENINFO, &ctx.orig_vi);
    CHECK(r, "ioctl(FBIOGET_VSCREENINFO)");
    memcpy(&ctx.vi, &ctx.orig_vi, sizeof(ctx.vi));

    ctx.vi.bits_per_pixel = 24;
    ctx.vi.xres_virtual = ctx.vi.xres;
    ctx.vi.yres_virtual = ctx.vi.yres * 2;
    r = ioctl(ctx.fbfd, FBIOPUT_VSCREENINFO, &ctx.vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    info("framebuffer: %" PRIu32 "x%" PRIu32", %" PRIu32 "bpp",
         ctx.vi.xres, ctx.vi.yres, ctx.vi.bits_per_pixel);

    r = ioctl(ctx.fbfd, FBIOGET_FSCREENINFO, &ctx.fi);
    CHECK(r, "ioctl(FBIOGET_FSCREENINFO)");

    ctx.fb = mmap(NULL, ctx.fi.smem_len,
                  PROT_READ | PROT_WRITE, MAP_SHARED, ctx.fbfd, 0);
    CHECK_NOT(ctx.fb, MAP_FAILED, "mmap(fbfd)");

    ctx.page_size = ctx.fi.line_length * ctx.vi.yres;
    memset(ctx.fb, 0, ctx.fi.smem_len);
}

void close_framebuffer(void)
{
    int r = ioctl(ctx.fbfd, FBIOPUT_VSCREENINFO, &ctx.orig_vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    r = close(ctx.fbfd); CHECK(r, "close(fbfd)");
    r = munmap(ctx.fb, ctx.fi.smem_len); CHECK(r, "munmap");
}


int main(int argc, char* argv[])
{
    initialize_context();

    midi_initialize(20, 0);
    mark_init(&ctx.syncs, "tempo", (double)60/24, "BPM", 100);
    mark_init(&ctx.frames, "fps", 1, "", 5);

    assert(argc == 2);
    open_framebuffer(argv[1]);

    while(1) {
        midi_next();
    }

    close_framebuffer();

    return 0;
}
