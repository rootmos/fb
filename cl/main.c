#include <r.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "rt.h"
#include "world.c"
#include "enc.c"

int main(int argc, const char* argv[])
{
    assert(argc == 2); const char* fn = argv[1];

    rt_initialize();

    const size_t fps = 24, duration = 10;
#ifndef DEBUG
#ifndef QUICK
    const size_t w = 1280, h = 720, samples = 1+8*8, frames = fps * duration;
#else
    const size_t w = 1280, h = 720, samples = 1, frames = fps * duration;
#endif
#else
    const size_t w = 1, h = 1, samples = 1, frames = 1;
#endif

    color_t* buf = enc_initialize(w, h, fps, fn);

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
