/* vim: set ft=c: */

ulong rnd(entropy_t* e, seed_t* s)
{
    return e->uniform[(*s)++ % e->uniform_N];
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

float normal_dist(entropy_t* e, seed_t* s)
{
    return e->normal[(*s)++ % e->normal_N];
}
