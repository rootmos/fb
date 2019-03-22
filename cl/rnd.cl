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

seed_t rnd_combine(seed_t xs[], size_t N)
{
    seed_t x = xs[0];
    for(size_t i = 0; i < N; i++) {
        x ^= xs[i] << 23;
        x ^= xs[i] >> 17;
    }
    return x;
}

float uniform_float(ulong x)
{
    return (float)x/ULONG_MAX;
}

seed_t seed_from_vec(vec_t v)
{
    return mad(1103515245, length(v), 12345);
}

float normal_dist(seed_t* s)
{
    float x, y, r2;
    do {
        x = 2*uniform_float(rnd(s)) - 1;
        y = 2*uniform_float(rnd(s)) - 1;
        r2 = x*x + y*y;
    } while(r2 > 1 || r2 == 0);

    return x * sqrt(-2*log(r2) / r2);
}
