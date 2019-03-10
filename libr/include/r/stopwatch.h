#pragma once

#include <stddef.h>

struct stopwatch;

struct stopwatch* stopwatch_mk(const char* const what, const size_t period);
void stopwatch_free(struct stopwatch* const s);

void stopwatch_reset(struct stopwatch* const s);
void stopwatch_tick(struct stopwatch* const s);
void stopwatch_start(struct stopwatch* const s);
void stopwatch_stop(struct stopwatch* const s);
