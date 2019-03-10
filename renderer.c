#include <r.h>

#include "renderer.h"
#include "demo.h"
#include "fb.h"

#include <sys/prctl.h>
#include <sys/eventfd.h>
#include <sys/signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include <poll.h>
#include <string.h>

struct renderer {
    struct state* st;
    const char* fbfn;
    pid_t pid;
    int efd;
    struct mark* frames;
};

static void renderer_main(const struct renderer* const ctx)
{
    fb_open(ctx->fbfn);

    struct pollfd fds[] = { { .fd = ctx->efd, .events = POLLIN } };
    while(1) {
        int r = poll(fds, 1, -1); CHECK(r, "poll");
        assert(fds[0].revents == POLLIN);

        demo_render(ctx->st);

        fb_flip();
        mark_tick(ctx->frames);

        eventfd_t v;
        r = eventfd_read(ctx->efd, &v); CHECK(r, "eventfd_read");
    }

    fb_close();
    exit(0);
}

void renderer_next(struct renderer* const ctx, const struct state* const st)
{
    memcpy(ctx->st, st, demo_state_size());
    int r = eventfd_write(ctx->efd, 0xfffffffffffffffe);
    CHECK(r, "eventfd_write");
}

struct pollfd renderer_pollfd(const struct renderer* const ctx)
{
    return (struct pollfd){ .fd = ctx->efd, .events = POLLOUT };
}

struct renderer* renderer_start(const char* const fbfn)
{
    struct renderer* ctx = (struct renderer*)calloc(sizeof(*ctx), 1);

    ctx->frames = mark_init("fps", 1, "", 30);
    ctx->fbfn = fbfn;

    ctx->st = mmap(NULL, demo_state_size(),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    CHECK_NOT(ctx->st, MAP_FAILED, "mmap");

    int p[2];
    int r = pipe(p); CHECK(r, "pipe");
    ctx->efd = eventfd(0, EFD_NONBLOCK); CHECK(ctx->efd, "eventfd");
    ctx->pid = fork(); CHECK(ctx->pid, "fork");
    if(ctx->pid == 0) {
        r = prctl(PR_SET_PDEATHSIG, SIGKILL);
        CHECK(r, "prctl(PR_SET_PDEATHSIG)");
        renderer_main(ctx);
    }
    r = close(p[0]); CHECK(r, "close(p[0])");
    info("renderer: pid=%d", ctx->pid);
    return ctx;
}
