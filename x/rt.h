#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t b, g, r, _;
} color_t;

#define color(rr,gg,bb) ((color_t){ .r = rr, .g = gg, .b = bb })

#define black color(0, 0, 0)
#define white color(0xff, 0xff, 0xff)
#define red color(0xff, 0x00, 0x00)
#define green color(0x00, 0xff, 0x00)
#define blue color(0x00, 0x00, 0xff)
#define violet color(0x7f, 0x00, 0xff)
#define orange color(0xff, 0x80, 0x00)

void rt_setup(void);
void rt_draw(color_t buf[], size_t height, size_t width);
