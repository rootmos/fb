#include <r.h>
#include "rt.h"

typedef struct {
    float x, y, z;
} vec_t;

#define vec(x, y, z) ((vec_t){ x, y, z })

// p + span(b)
typedef struct {
    vec_t p;
    vec_t b;
} line_t;

// forall v: (v - p) . n = 0
typedef struct {
    vec_t p;
    vec_t n;
} plane_t;

// p + span(b_0, b_1)
typedef struct {
    vec_t p;
    vec_t b[2];
} grid_t;

// |v - c| = r
typedef struct {
    vec_t c;
    float r;
} sphere_t;

vec_t add(vec_t v, vec_t w)
{
    return (vec_t){ .x = v.x + w.x, .y = v.y + w.y, .z = v.z + w.z };
}

vec_t sub(vec_t v, vec_t w)
{
    return (vec_t){ .x = v.x - w.x, .y = v.y - w.y, .z = v.z - w.z };
}

float dot(vec_t v, vec_t w)
{
    return v.x*w.x + v.y*w.y + v.z*w.z;
}

vec_t scalar_prod(float t, vec_t v)
{
    return (vec_t){ .x = t * v.x, .y = t * v.y, .z = t * v.z};
}

line_t line_from_two_points(vec_t v, vec_t w)
{
    return (line_t){ .p = v, .b = sub(w, v) };
}

vec_t project_point_on_line(line_t l, vec_t v)
{
    return add(l.p, scalar_prod(dot(l.b, sub(v, l.p))/dot(l.b, l.b), l.b));
}

vec_t coordinates(vec_t origin, vec_t base[], float t[], size_t dim)
{
    for(size_t i = 0; i < dim; i++) {
        origin = add(origin, scalar_prod(t[i], base[i]));
    }
    return origin;
}

vec_t line_coord(line_t l, float t)
{
    return coordinates(l.p, &l.b, &t, 1);
}

vec_t grid_coord(grid_t g, float t[])
{
    return coordinates(g.p, g.b, t, 2);
}

// at^2 + bt + c = 0
int solve_2nd_order(float a, float b, float c, float t[])
{
    const float d = b*b - 4*a*c;
    if(d < 0) return 0;

    const float p = -b/(2*a);
    t[0] = p + d;
    t[1] = p - d;
    return 2;
}

int intersect_line_sphere(line_t l, sphere_t s, float t[])
{
    return solve_2nd_order(
        dot(l.b, l.b),
        -2*dot(l.b, s.c),
        dot(sub(l.p, s.c), sub(l.p, s.c)) - s.r*s.r,
        t
    );
}

typedef struct {
    vec_t camera;
    grid_t plane;
} viewport_t;

typedef struct {
    sphere_t* spheres;
    size_t spheres_len;
} world_t;

#define black ((color_t){ 0 })
#define white ((color_t){ 0xff })

static viewport_t view;
static world_t world;

void rt_setup(void)
{
    view.camera = vec(-10.0, 0, 0);
    view.plane.p = vec(0, 0, 0);
    view.plane.b[0] = vec(0, 0.1, 0);
    view.plane.b[1] = vec(0, 0, 0.1);

    static sphere_t ss[] = {
        { .c = vec(20, 0, 0), .r = 5 },
    };
    world.spheres = ss;
    world.spheres_len = LENGTH(ss);
}

void rt_draw(color_t buf[], size_t height, size_t width)
{
    for(size_t i = 0; i < height; i++) {
        for(size_t j = 0; j < width; j++) {
            vec_t p = grid_coord(view.plane, (float[]){ i - (height/2), j - (width/2) });
            line_t l = line_from_two_points(view.camera, p);
            trace("checking line: p=(%f,%f,%f) b=(%f,%f,%f)",
                  l.p.x, l.p.y, l.p.z,
                  l.b.x, l.b.y, l.b.z);

            for(size_t n = 0; n < world.spheres_len; n++) {
                float t[2];
                int r = intersect_line_sphere(l, world.spheres[n], t);
                if(r == 0) {
                    buf[i*width + j] = black;
                } else {
                    debug("drawing sphere %zu, %d intersections", n, r);
                    if(t[0] > 0 || t[1] > 0) {
                        buf[i*width + j] = white;
                    } else {
                        buf[i*width + j] = black;
                    }
                }
            }
        }
    }
}
