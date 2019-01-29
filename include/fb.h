#pragma once

#include <stddef.h>
#include <stdint.h>

void fb_open(const char* const fbfn);
void fb_close(void);

void fb_flip(void);

struct fb_info {
    size_t xres;
    size_t yres;
};

const struct fb_info fb_info(void);

typedef uint8_t color_t;
void fb_clear(const color_t c);
void fb_pixel(const size_t x, const size_t y, const color_t c);
void fb_rect(const size_t x, const size_t y, const size_t h, const size_t w,
             const color_t c);
