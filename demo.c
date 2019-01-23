#include "util.h"

#include <linux/fb.h>
#include <linux/ioctl.h>
#include <stropts.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <alsa/asoundlib.h>

struct fb_fix_screeninfo fi;
struct fb_var_screeninfo vi;
int fbfd = -1;
char* fb = NULL;
size_t page_size = 0;
uint_fast8_t cur_page = 0;

struct context {
    snd_seq_t* seq;
};

struct context ctx;

static void clear(uint_fast8_t page)
{
    memset(fb + page_size * page, 0, page_size);
}

typedef uint_fast32_t color_t;

static void put_pixel(const size_t x, const size_t y, const color_t c)
{
    const size_t off = (x * 3) + y * fi.line_length + cur_page * page_size;
    memcpy(fb + off, &c, 3);
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

static void initialize_alsa(void)
{
    int r = snd_seq_open(&ctx.seq, "default", SND_SEQ_OPEN_INPUT, 0);
    if(r != 0) { failwith("unable to open ALSA sequencer"); }

    int client_id = snd_seq_client_id(ctx.seq);
    info("client_id=%d", client_id);
}

int main(int argc, char* argv[])
{
    initialize_alsa();

    assert(argc == 2);
    const char* const fbfn = argv[1];
    fbfd = open(fbfn, O_RDWR); CHECK(fbfd, "open(%s, O_RDWR)", fbfn);

    struct fb_var_screeninfo orig_vi;
    int r = ioctl(fbfd, FBIOGET_VSCREENINFO, &orig_vi);
    CHECK(r, "ioctl(FBIOGET_VSCREENINFO)");
    memcpy(&vi, &orig_vi, sizeof(vi));

    vi.bits_per_pixel = 24;
    vi.xres_virtual = vi.xres;
    vi.yres_virtual = vi.yres * 2;
    r = ioctl(fbfd, FBIOPUT_VSCREENINFO, &vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    info("framebuffer: %" PRIu32 "x%" PRIu32", %" PRIu32 "bpp",
         vi.xres, vi.yres, vi.bits_per_pixel);

    r = ioctl(fbfd, FBIOGET_FSCREENINFO, &fi);
    CHECK(r, "ioctl(FBIOGET_FSCREENINFO)");

    fb = mmap(NULL, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    CHECK_NOT(fb, MAP_FAILED, "mmap(fbfd)");

    page_size = fi.line_length * vi.yres;
    memset(fb, 0, fi.smem_len);

    for(size_t i = 0; i < 300; i += 100) {
        clear(cur_page);
        draw_rect(0   + i, 0  , 300, 300, 0xff0000);
        draw_rect(300 + i, 300, 300, 300, 0x00ff00);
        draw_rect(600 + i, 600, 300, 300, 0x0000ff);

        vi.yoffset = cur_page * vi.yres;
        vi.activate = FB_ACTIVATE_VBL;
        r = ioctl(fbfd, FBIOPAN_DISPLAY, &vi);
        CHECK(r, "ioctl(FBIOPAN_DISPLAY)");
        r = ioctl(fbfd, FBIO_WAITFORVSYNC, NULL);
        CHECK(r, "ioctl(FBIO_WAITFORVSYNC)");

        cur_page = (cur_page + 1) % 2;
    }

    r = ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    r = close(fbfd); CHECK(r, "close(fbfd)");
    r = munmap(fb, fi.smem_len); CHECK(r, "munmap");

    return 0;
}
