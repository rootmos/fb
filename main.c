#include <alsa/asoundlib.h>

#include <r.h>
#include "mark.h"
#include "fb.h"
#include "demo.h"
#include "renderer.h"

#include <assert.h>

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
