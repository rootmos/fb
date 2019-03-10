#pragma once

#include <stddef.h>

struct mark* mark_init(const char* const what,
                       const double factor,
                       const char* const unit,
                       const size_t check_period);
void mark_free(struct mark* const m);

void mark_tick(struct mark* const m);
void mark_set(struct mark* const m);
