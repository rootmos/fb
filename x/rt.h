#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t b, g, r, _;
} color_t;

#define color(rr,gg,bb) ((color_t){ .r = rr, .g = gg, .b = bb })

#define black ((color_t){ 0 })
#define white ((color_t){ 0xff })

#define red color(0xff, 0x00, 0x00)
#define green color(0x00, 0xff, 0x00)
#define blue color(0x00, 0x00, 0xff)

void rt_setup(void);
void rt_draw(color_t buf[], size_t height, size_t width);
