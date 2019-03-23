#include <r.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "rt.h"
#include "world.c"
#include "enc.c"

int main(int argc, char** argv)
{
    rt_initialize();

    const size_t fps = 24, duration = 15;
#ifndef DEBUG
#ifndef QUICK
    const size_t w = 1920, h = 1080, samples = 1+15*15, frames = fps * duration;
#else
    const size_t w = 1280, h = 720, samples = 1, frames = fps * duration;
#endif
#else
    const size_t w = 1, h = 1, samples = 1, frames = 1;
#endif

    color_t* buf = enc_initialize(w, h, "out.mkv");

    for(size_t i = 0; i < frames; i++) {
        world_t* world = create_world(i, duration, fps);
        rt_draw(world, w, h, samples, buf);
        free(world);
#ifndef DEBUG
        enc(i);
#endif
    }

    enc_finalize(1);

    return 0;
}
