#pragma once

#include <CL/cl.h>

typedef struct { uint8_t r, g, b; } color_t;
typedef cl_float3 vec_t;
typedef uint64_t seed_t;
typedef uint64_t probability_t;

#define PROB_ALWAYS ((probability_t)UINT64_MAX)
#define PROB_NEVER ((probability_t)0)

#define vec(xx,yy,zz) (((cl_float3){ .s = { xx, yy, zz } }))
