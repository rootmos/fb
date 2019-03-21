#pragma once

#include "types.h"
#include "shared.h"

void rt_write_ppm(int fd, const color_t buf[], size_t width, size_t height);

void rt_initialize(void);
void rt_run(void);
void rt_draw(const world_t* w, size_t width, size_t height, size_t samples, color_t buf[]);
