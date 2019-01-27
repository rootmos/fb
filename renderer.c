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
    pid_t pid;
    int efd;
};

static void render(const struct state* const st)
{
    fb_rect(0   + 5*st->ticks, 0  , 300, 300, 0xff0000);
    fb_rect(300 + 5*st->ticks, 300, 300, 300, 0x00ff00);
    fb_rect(600 + 5*st->ticks, 600, 300, 300, 0x0000ff);
}

static void renderer_main(const struct renderer* const ctx)
{
    struct pollfd fds[] = { { .fd = ctx->efd, .events = POLLIN } };
    while(1) {
        int r = poll(fds, 1, 0); CHECK(r, "poll");
        assert(fds[0].revents == POLLIN);

        render(ctx->st);

        eventfd_t v;
        r = eventfd_read(ctx->efd, &v); CHECK(r, "eventfd_read");
    }

    exit(0);
}

void renderer_next(struct renderer* const ctx, const struct state* const st)
{
    memcpy(ctx->st, st, sizeof(*st));
    int r = eventfd_write(ctx->efd, 0xfffffffffffffffe);
    CHECK(r, "eventfd_write");
}

struct renderer* renderer_start(void)
{
    struct renderer* ctx = (struct renderer*)calloc(sizeof(*ctx), 1);

    ctx->st = mmap(NULL, sizeof(struct state),
                   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    CHECK_NOT(ctx->st, MAP_FAILED, "mmap");

    int p[2];
    int r = pipe(p); CHECK(r, "pipe");
    ctx->efd = eventfd(0, 0); CHECK(ctx->efd, "eventfd");
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
