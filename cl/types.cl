/* vim: set ft=c: */

typedef struct { uchar r, g, b; } color_t;
typedef float3 vec_t;
typedef ulong seed_t;
typedef ulong probability_t;

#define vec(x, y, z) ((vec_t){ x, y, z })
