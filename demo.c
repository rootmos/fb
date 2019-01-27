#include <alsa/asoundlib.h>

#include <r.h>
#include "mark.h"
#include "fb.h"

#include <assert.h>
#include <inttypes.h>
#include <sys/eventfd.h>
#include <sys/prctl.h>
#include <sys/signal.h>

struct {
    snd_seq_t* seq;
    int seq_input_port;
    size_t sync;

    struct mark* frames;
    struct mark* syncs;
} ctx;

static void initialize_context(void)
{
    memset(&ctx, 0, sizeof(ctx));
}

static void midi_initialize(const int src_client, const int src_port)
{
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
    fb_rect(0   + 5*ctx.sync, 0  , 300, 300, 0xff0000);
    fb_rect(300 + 5*ctx.sync, 300, 300, 300, 0x00ff00);
    fb_rect(600 + 5*ctx.sync, 600, 300, 300, 0x0000ff);
}

static void renderer_main(const int efd)
{
    eventfd_t v;
    int r = eventfd_read(efd, &v); CHECK(r, "eventfd_read");
    info("event: %" SCNu64, v);

    exit(0);
}

void renderer_start(void)
{
    int p[2];
    int r = pipe(p); CHECK(r, "pipe");
    int efd = eventfd(0, 0); CHECK(efd, "eventfd");
    const pid_t pid = fork(); CHECK(pid, "fork");
    if(pid == 0) {
        r = prctl(PR_SET_PDEATHSIG, SIGKILL);
        CHECK(r, "prctl(PR_SET_PDEATHSIG)");
        renderer_main(efd);
    } else {
        r = close(p[0]); CHECK(r, "close(p[0])");
        info("renderer: pid=%d", pid);
    }
}

void midi_next(void)
{
    snd_seq_event_t* ev;
    int r = snd_seq_event_input(ctx.seq, &ev);
    CHECK_ALSA(r, "unable to retrieve input command");

    switch(ev->type) {
    case SND_SEQ_EVENT_CLOCK:
        mark_tick(ctx.syncs);

        if(ctx.sync % 24 == 0) {
            /*render();*/
            mark_tick(ctx.frames);
        }
        ctx.sync += 1;
        break;

    case SND_SEQ_EVENT_NOTEON:
    default: break;
    }

    r = snd_seq_free_event(ev);
    CHECK_ALSA(r, "unable to free event");
}

int main(int argc, char* argv[])
{
    initialize_context();
    renderer_start();

    midi_initialize(20, 0);

    ctx.syncs = mark_init("tempo", (double)60/24, "BPM", 100);
    ctx.frames = mark_init("fps", 1, "", 5);

    assert(argc == 2);
    fb_open(argv[1]);

    while(1) {
        midi_next();
    }

    fb_close();

    return 0;
}
