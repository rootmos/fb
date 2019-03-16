#include "r/stopwatch.h"
#include "r/fail.h"
#include "r/logging.h"

#include <assert.h>
#include <time.h>
#include <stdlib.h>

struct stopwatch {
    const char* what;
    size_t period;

    int running;
    struct timespec t;
    struct timespec acc;
    size_t intervals;
    size_t ticks;
};

void stopwatch_reset(struct stopwatch* const s)
{
    s->running = 0;
    s->ticks = 0;
    s->intervals = 0;
    s->t = (struct timespec){ 0 };
}

struct stopwatch* stopwatch_mk(const char* const what,
                               const size_t period)
{
    struct stopwatch* s = calloc(sizeof(*s), 1); assert(s);
    s->what = what;
    s->period = period;

    stopwatch_reset(s);
    return s;
}

void stopwatch_free(struct stopwatch* const s)
{
    free(s);
}


void stopwatch_tick(struct stopwatch* const s)
{
    if(++s->ticks == s->period)
    {
        const double secs = (double)(s->acc.tv_sec) + ((double)(s->acc.tv_nsec))/1000000000;
        const double avg = secs / s->intervals;
        info("%s: avg=%f", s->what, avg);
        s->acc = (struct timespec){ 0 };
        s->intervals = 0;
        s->ticks = 0;
    }
}

void stopwatch_start(struct stopwatch* const s)
{
    if(!s->running) {
        int r = clock_gettime(CLOCK_MONOTONIC_RAW, &s->t);
        CHECK(r, "clock_gettime(CLOCK_MONOTONIC_RAW, ..)");

        s->running = 1;
    }
}

void stopwatch_stop(struct stopwatch* const s)
{
    if(s->running) {
        s->intervals++;
        s->running = 0;

        struct timespec now;
        int r = clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        CHECK(r, "clock_gettime(CLOCK_MONOTONIC_RAW, ..)");

        s->acc.tv_sec += now.tv_sec - s->t.tv_sec;
        s->acc.tv_nsec += now.tv_nsec - s->t.tv_nsec;
    }

    stopwatch_tick(s);
}
