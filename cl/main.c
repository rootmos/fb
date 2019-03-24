#include <CL/cl.h>
#include <r.h>

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "types.h"
#include "shared.h"
#include "world.c"
#include "enc.c"
#include "rt.c"

int main(int argc, const char* argv[])
{
    assert(argc == 2); const char* fn = argv[1];

    const size_t fps = 24, duration = 15;
#if defined(DEBUG)
    const size_t w = 2, h = 2, samples = 1, frames = 1;
#elif defined(QUICK)
    const size_t w = 1280, h = 720, samples = 1+4*4, frames = fps * duration;
#else
    const size_t w = 1920, h = 1080, samples = 1+8*8, frames = fps * duration;
#endif

    const char* fmt = fn + strlen(fn) - 3;
    if(!fmt || strcmp(fmt, "ppm") == 0) {
        rt_initialize(1);

        color_t buf[w*h];
        world_t* world = create_world(0, duration, fps);
        rt_draw(world, w, h, samples, buf);
        free(world);

        int fd = open(fn, O_CREAT | O_WRONLY,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        CHECK(fd, "open(%s)", fn);

        rt_write_ppm_header(fd, w, h);
        rt_write_raw(fd, buf, w, h);

        int r = close(fd); CHECK(r, "close");
    } else if(strcmp(fmt, "mkv") == 0) {
        rt_initialize(fps);

        color_t* buf = enc_initialize(w, h, fps, fn);
        for(size_t i = 0; i < frames; i++) {
            info("rendering frame %zu/%zu", i, frames);
            world_t* world = create_world(i, duration, fps);
            rt_draw(world, w, h, samples, buf);
            free(world);
            enc(i);
        }
        enc_finalize();
    }

    rt_deinitialize();

    return 0;
}
