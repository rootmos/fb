#include <r.h>

#include <string.h>

#include "rt.h"
#include "world.c"

int main(int argc, char** argv)
{
    rt_initialize();

#ifndef DEBUG
#ifndef QUICK
    const size_t w = 1920, h = 1080, samples = 100;
#else
    const size_t w = 1920, h = 1080, samples = 10;
#endif
#else
    const size_t w = 1, h = 1, samples = 1;
#endif

    color_t buf[w*h];
    rt_draw(create_world(), w, h, samples, buf);
    rt_run();

#ifndef DEBUG
    rt_write_ppm(1, buf, w, h);
#endif

    return 0;
}
