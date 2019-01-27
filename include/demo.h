#pragma once

#include <alsa/asoundlib.h>
#include <stddef.h>

struct state;

size_t demo_state_size(void);
struct state* demo_fresh_state(void);

void demo_start(struct state* const st);
void demo_stop(struct state* const st);
void demo_continue(struct state* const st);

void demo_tick(struct state* const st);
void demo_note_on(struct state* const st, const snd_seq_ev_note_t* const node);
void demo_note_off(struct state* const st, const snd_seq_ev_note_t* const node);
void demo_ctrl(struct state* const st, const snd_seq_ev_ctrl_t* const ctrl);

void demo_render(const struct state* const st);
