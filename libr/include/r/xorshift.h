#pragma once

#include <stddef.h>
#include <stdint.h>

void random_bytes(void* buf, size_t n);

static inline uint64_t xorshift64(uint64_t* state)
{
    uint64_t x = *state;
    x^= x << 13;
    x^= x >> 7;
    x^= x << 17;
    *state = x;
    return x;
}

static inline uint64_t xorshift128plus(uint64_t* state)
{
    uint64_t x = state[0];
    uint64_t const y = state[1];
    state[0] = y;
    x ^= x << 23; // a
    state[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c
    return state[1] + y;
}

extern uint64_t xorshift_state[2];
void xorshift_state_initalize(void);
inline static uint64_t xorshift64_i(void) { return xorshift64(xorshift_state); }
inline static uint64_t xorshift128plus_i(void) { return xorshift128plus(xorshift_state); }

static inline float normal_dist(uint64_t* seed)
{
    float sum = 0; const size_t N = 6;
    for(size_t i = 0; i < N; i++) {
        uint64_t i = xorshift64(seed);
        sum += (float)(int64_t)i/INT64_MAX;
    }
    return sum/N;
}
