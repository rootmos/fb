#pragma once

#include <stddef.h>

struct state {
    size_t ticks;
};

void demo_tick(struct state* const st);
