#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t b, g, r, _;
} color_t;

void rt_setup(void);
void rt_draw(color_t buf[], size_t height, size_t width);
