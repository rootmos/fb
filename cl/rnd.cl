/* vim: set ft=c: */

seed_t rnd(seed_t* seed)
{
    // xorshift64
    seed_t x = *seed;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *seed = x;
    return x;
}

float uniform_float(ulong x)
{
    return (float)x/ULONG_MAX;
}
