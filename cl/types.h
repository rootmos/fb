#pragma once

#include <CL/cl.h>

typedef struct { uint8_t r, g, b; } color_t;
typedef cl_float3 vec_t;
typedef uint64_t seed_t;

#define vec(xx,yy,zz) (((cl_float3){ .s = { xx, yy, zz } }))
