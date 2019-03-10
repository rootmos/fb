#include <alsa/asoundlib.h>

#include <r.h>

#include "demo.h"
#include "renderer.h"

#include <assert.h>

struct {
    snd_seq_t* seq;
    int seq_input_port;

    struct state* st;
    struct renderer* renderer;

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

    r = snd_seq_nonblock(ctx.seq, 1); CHECK_ALSA(r, "snd_seq_nonblock");

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
    while(1) {
        snd_seq_event_t* ev;
        int r = snd_seq_event_input(ctx.seq, &ev);
        if(r == -EAGAIN) { break; }
        CHECK_ALSA(r, "unable to retrieve input command");

        switch(ev->type) {
        case SND_SEQ_EVENT_CLOCK:
            mark_tick(ctx.syncs);
            demo_tick(ctx.st);
            break;

        case SND_SEQ_EVENT_NOTEON:
            demo_note_on(ctx.st, &ev->data.note);
            break;

        case SND_SEQ_EVENT_NOTEOFF:
            demo_note_off(ctx.st, &ev->data.note);
            break;

        case SND_SEQ_EVENT_START:
            mark_set(ctx.syncs);
            demo_start(ctx.st);
            break;

        case SND_SEQ_EVENT_STOP:
            demo_stop(ctx.st);
            break;

        case SND_SEQ_EVENT_CONTINUE:
            mark_set(ctx.syncs);
            demo_continue(ctx.st);
            break;

        case SND_SEQ_EVENT_CONTROLLER:
            demo_ctrl(ctx.st, &ev->data.control);
            break;

        default: break;
        }

        r = snd_seq_free_event(ev); CHECK_ALSA(r, "unable to free event");
    }
}

void do_poll(void)
{
    struct pollfd fds[1 + snd_seq_poll_descriptors_count(ctx.seq, POLLIN)];

    fds[0] = renderer_pollfd(ctx.renderer);
    int r = snd_seq_poll_descriptors(ctx.seq, &fds[1], LENGTH(fds) - 1, POLLIN);
    CHECK_ALSA(r, "snd_seq_poll_descriptors");

    r = poll(fds, LENGTH(fds), -1); CHECK(r, "poll");

    // Check midi events
    unsigned short evs;
    snd_seq_poll_descriptors_revents(ctx.seq, &fds[1], LENGTH(fds) - 1, &evs);
    if(evs == POLLIN) {
        midi_next();
    }

    // Check on renderer
    if(fds[0].revents == POLLOUT) {
        renderer_next(ctx.renderer, ctx.st);
    }
}

int main(int argc, char* argv[])
{
    initialize_context();

    assert(argc == 2);
    ctx.renderer = renderer_start(argv[1]);
    ctx.st = demo_fresh_state();

    midi_initialize(20, 0);

    ctx.syncs = mark_init("tempo", (double)60/24, "BPM", 200);

    while(1) {
        do_poll();
    }

    return 0;
}
