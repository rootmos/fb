#include <r.h>
#include "demo.h"
#include "fb.h"

struct state {
    size_t ticks;
};

struct state* demo_fresh_state(void)
{
    struct state* st = (struct state*)calloc(sizeof(*st), 1); assert(st);
    return st;
}

size_t demo_state_size(void)
{
    return sizeof(struct state);
}

void demo_tick(struct state* const st)
{
    if(st->ticks > 0) {
        st->ticks -= 5;
    }
}

void demo_start(struct state* const st)
{
    info("start");
}

void demo_stop(struct state* const st)
{
    info("stop");
}

void demo_continue(struct state* const st)
{
    info("continue");
}

void demo_note_on(struct state* const st, const snd_seq_ev_note_t* const note)
{
    if(note->channel == 9) {
        st->ticks = 100;
    } else {
        info("on: channel=%u note=%u", note->channel, note->note);
    }
}

void demo_note_off(struct state* const st, const snd_seq_ev_note_t* const note)
{
    if(note->channel == 9) {
    } else {
        info("off: channel=%u note=%u", note->channel, note->note);
    }
}

void demo_ctrl(struct state* const st, const snd_seq_ev_ctrl_t* const ctrl)
{
    // interpret beatstep's control mode knobs and buttons
    if(ctrl->channel == 0) {
        if(ctrl->param > 15 && ctrl->param <= 31) {
            const unsigned char knob = ctrl->param - 15;
            info("knob %u: %d", knob, ctrl->value);
            return;
        } else if(ctrl->param > 35 && ctrl->param <= 51) {
            const unsigned char button = ctrl->param - 35;
            if(ctrl->value == 127) {
                info("button %u ON", button);
                return;
            } else if(ctrl->value == 0) {
                info("button %u OFF", button);
                return;
            }
        }
    }

    info("ctrl %u: %u=%d", ctrl->channel, ctrl->param, ctrl->value);
}

void demo_render(const struct state* const st)
{
    fb_rect(0   + 5*st->ticks, 0  , 300, 300, 0xff0000);
    fb_rect(300 + 5*st->ticks, 300, 300, 300, 0x00ff00);
    fb_rect(600 + 5*st->ticks, 600, 300, 300, 0x0000ff);
}
