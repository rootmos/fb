#pragma once

#include "demo.h"

struct renderer* renderer_start(const char* const fbfn);
struct pollfd renderer_pollfd(const struct renderer* const ctx);
void renderer_next(struct renderer* const ctx, const struct state* const st);
