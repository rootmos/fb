/* vim: set ft=c: */

typedef struct { uchar r, g, b; } color_t;
typedef float3 vec_t;
typedef ulong seed_t;

#define vec(x, y, z) ((vec_t){ x, y, z })

typedef struct {
    ulong uniform_N;
    __constant ulong* uniform;

    ulong normal_N;
    __constant float* normal;
} entropy_t;
