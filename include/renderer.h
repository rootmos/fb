#pragma once

#include "demo.h"

struct renderer* renderer_start(void);
void renderer_next(struct renderer* const ctx, const struct state* const st);
