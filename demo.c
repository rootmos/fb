#include <r.h>
#include "demo.h"
#include "fb.h"

#define KNOB(k, scale) ((scale)*st->knob[(k) - 1]/127)
#define BUTTON(b) (st->button[(b) - 1])

struct state {
    size_t ticks;
    size_t bars;
    size_t qn;

    unsigned button[16];
    unsigned char knob[16];
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
    st->ticks += 1;
    if(st->ticks % 24 == 0) {
        st->qn += 1;
    }
    if(st->qn == 4) {
        st->qn = 0;
        st->bars += 1;
        info("new bar");
    }

    if(st->bars == 2) exit(0);
}

void demo_start(struct state* const st)
{
    info("start");
    st->ticks = 0;
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
    info("on: channel=%u note=%u", note->channel, note->note);
}

void demo_note_off(struct state* const st, const snd_seq_ev_note_t* const note)
{
    info("off: channel=%u note=%u", note->channel, note->note);
}

void demo_ctrl(struct state* const st, const snd_seq_ev_ctrl_t* const ctrl)
{
    // interpret beatstep's control mode knobs and buttons
    if(ctrl->channel == 0) {
        if(ctrl->param > 15 && ctrl->param <= 31) {
            st->knob[ctrl->param - 16] = ctrl->value;
            return;
        } else if(ctrl->param > 35 && ctrl->param <= 51) {
            if(ctrl->value == 127) {
                st->button[ctrl->param - 36] = 1;
            } else if(ctrl->value == 0) {
                st->button[ctrl->param - 36] = 0;
            }
            return;
        }
    }

    info("ctrl %u: %u=%d", ctrl->channel, ctrl->param, ctrl->value);
}

void demo_render(const struct state* const st)
{
    struct fb_info fbi = fb_info();

    const color_t c = 3;
    const size_t h = fbi.yres / 3;
    const size_t w = fbi.xres / 6;

    fb_clear(0);
    fb_rect(0 + st->ticks, 0, h, w, c);
    fb_rect(w + st->ticks, h, h, w, c);
    fb_rect(2*w + st->ticks, 2*h, h, w, c);
}
