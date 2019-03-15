#include <r.h>
#include "rt.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>

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

inline static __attribute__((always_inline))
vec_t add(vec_t v, vec_t w)
{
    return (vec_t){ .x = v.x + w.x, .y = v.y + w.y, .z = v.z + w.z };
}

inline static __attribute__((always_inline))
vec_t sub(vec_t v, vec_t w)
{
    return (vec_t){ .x = v.x - w.x, .y = v.y - w.y, .z = v.z - w.z };
}

inline static __attribute__((always_inline))
float dot(vec_t v, vec_t w)
{
    return v.x*w.x + v.y*w.y + v.z*w.z;
}

inline static __attribute__((always_inline))
vec_t cross(vec_t a, vec_t b)
{
    return vec(a.y*b.z - a.z*b.y, - (a.x*b.z - a.z*b.x), a.x*b.y - a.y*b.x);
}

inline static __attribute__((always_inline))
float norm_sq(vec_t v)
{
    return v.x*v.x + v.y*v.y + v.z*v.z;
}

inline static __attribute__((always_inline))
float norm(vec_t v)
{
    return sqrtf(norm_sq(v));
}

inline static __attribute__((always_inline))
vec_t scalar_prod(float t, vec_t v)
{
    return (vec_t){ .x = t * v.x, .y = t * v.y, .z = t * v.z};
}

inline static __attribute__((always_inline))
vec_t normalize(vec_t v)
{
    return scalar_prod(1/norm(v), v);
}

inline static __attribute__((always_inline))
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

inline static __attribute__((always_inline))
vec_t grid_coord(grid_t g, float s, float t)
{
    return vec(
        g.p.x + s*g.b[0].x + t*g.b[1].x,
        g.p.y + s*g.b[0].y + t*g.b[1].y,
        g.p.z + s*g.b[0].z + t*g.b[1].z
    );
}

inline static __attribute__((always_inline))
vec_t line_coord(line_t l, float t)
{
    return add(l.p, scalar_prod(t, l.b));
}

#define eqf(a,b) (fabsf(a - b) < 1e-8)

// at^2 + bt + c = 0
int solve_2nd_order(float a, float b, float c, float t[])
{
    if(eqf(a, 0)) {
        t[0] = -c/b;
        return 1;
    }

    float d = b*b - 4*a*c;
    if(d < 0) return 0;

    const float p = b/(-2*a);
    d = sqrtf(d) / (2*a);
    t[0] = p + d;
    t[1] = p - d;
    return 2;
}

void solve_2nd_order_tests(void)
{
    float t[2];
    int r = solve_2nd_order(1, 0, -4, t);
    assert(r == 2);
    assert(eqf(t[0], 2));
    assert(eqf(t[1], -2));

    r = solve_2nd_order(1, 0, 4, t);
    assert(r == 0);
}

int intersect_line_sphere(line_t l, sphere_t s)
{
    const vec_t a = sub(s.c, l.p);
    const vec_t b = l.b; // normalized
    return s.r*s.r > norm_sq(a) - (dot(a, b)*dot(a, b));
}

int intersect_line_sphere_points(const line_t* l, const sphere_t* s,
                                 float t[])
{
    const vec_t d = sub(l->p, s->c);
    return solve_2nd_order(
        norm_sq(l->b),
        2*dot(l->b, d),
        norm_sq(d) - s->r*s->r,
        t
    );
}

void intersect_line_sphere_points_tests(void)
{
    line_t l = { .p = vec(-3, 0, 0), .b = vec(1, 0, 0) };
    sphere_t s = { .c = vec(10, 0, 0), .r = 5 };

    float t[2];
    int r = intersect_line_sphere_points(&l, &s, t);
    assert(r == 2);
    assert(eqf(t[0], 18));
    assert(eqf(t[1], 8));
}

int intersect_line_plane(const line_t* l, const plane_t* p, float t[])
{
    const float u = dot(sub(p->p, l->p), p->n);
    if(eqf(u, 0)) return t[0] = 0, 1; // line is in the plane
    const float v = dot(l->b, p->n);
    if(eqf(v, 0)) return 0; // line is parallel
    return t[0] = u / v, 1;
}

typedef struct {
    color_t color;
    color_t light;
} material_t;

typedef enum {
    SHAPE_TYPE_SPHERE,
    SHAPE_TYPE_PLANE,
} shape_type_t;

typedef struct {
    shape_type_t shape_type;
    union {
        sphere_t sphere;
        plane_t plane;
    } shape;
    material_t material;
} object_t;

int intersect_line_object(const line_t* l, const object_t* o, float t[])
{
    switch(o->shape_type) {
    case SHAPE_TYPE_SPHERE:
        return intersect_line_sphere_points(l, &o->shape.sphere, t);
    case SHAPE_TYPE_PLANE:
        return intersect_line_plane(l, &o->shape.plane, t);
    default:
        failwith("unsupported shape");
    }
}

typedef struct {
    vec_t camera;
    grid_t plane;
} viewport_t;

typedef struct {
    object_t* objects;
    size_t objects_len;
} world_t;

static viewport_t view;
static world_t world;
static struct stopwatch* stopwatch;

void rt_setup(void)
{
    solve_2nd_order_tests();
    intersect_line_sphere_points_tests();

    view.camera = vec(-10.0, 0, 5);
    view.plane.p = vec(0, 0, 5);
    view.plane.b[0] = vec(0, 0.01, 0);
    view.plane.b[1] = vec(0, 0, -0.01);

    static object_t os[] = {
        {
            .shape_type = SHAPE_TYPE_SPHERE,
            .shape.sphere = { .c = vec(10, 0, 5), .r = 5 },
            .material.color = green,
        },
        {
            .shape_type = SHAPE_TYPE_PLANE,
            .shape.plane = { .p = vec(0, 0, 0), .n = vec(0, 0, 1) },
            .material.color = red,
        },
    };
    world.objects = os;
    world.objects_len = LENGTH(os);

    stopwatch = stopwatch_mk("rt_draw", 1);
}

const object_t* find_collision(const line_t* l, const world_t* w, float* t)
{
    float t_min = -1; size_t n = w->objects_len;
    for(size_t i = 0; i < w->objects_len; i++) {
        float s[2];
        int r = intersect_line_object(l, &world.objects[i], s);

        for(size_t j = 0; j < r; j++) {
            if(s[0] < 0 || s[j] < s[0]) s[0] = s[j];
        }

        if(r > 0 && s[0] >= 0 && (t_min < 0 || s[0] < t_min)) {
            t_min = s[0]; n = i;
        }
    }

    if(n < w->objects_len) {
        if(t != NULL) {
            *t = t_min;
        }
        return &w->objects[n];
    } else {
        return NULL;
    }
}

typedef struct {
    material_t m;
} ray_collision_t;

static ray_collision_t sky_collision(const line_t* l)
{
    return (ray_collision_t) { .m.light = blue };
}

color_t color_mix(color_t factor, color_t c)
{
    return color(c.r*factor.r/0xff, c.g*factor.g/0xff, c.b*factor.b/0xff);
}

color_t color_add(color_t x, color_t y)
{
    return color(x.r+y.r, x.g+y.g, x.b+y.b);
}

vec_t rotate(float angle, vec_t axis, vec_t v)
{
    not_implemented();
}

float angle(vec_t a, vec_t b)
{
    return acosf(dot(a, b)/(norm(a)*norm(b)));
}

// pre-conditions: l->p is in the surface of o->shape
line_t reflect_line_object(const line_t* l, const object_t* o)
{
    vec_t n;
    switch(o->shape_type) {
    case SHAPE_TYPE_SPHERE:
        n = sub(l->p, o->shape.sphere.c);
        break;
    case SHAPE_TYPE_PLANE:
        n = o->shape.plane.n;
        break;
    default:
        failwith("unsupported shape");
    }

    return (line_t) {
        .p = l->p,
        .b = rotate(2*angle(l->b, n), cross(l->b, n), l->b)
    };
}

color_t ray_trace(const world_t* w, const line_t* line, size_t N)
{
    ray_collision_t cs[N];

    line_t l = *line;
    size_t n = 0; for(; n < N; n++) {
        float t;
        const object_t* o = find_collision(&l, w, &t);
        if(!o) {
            cs[n] = sky_collision(&l);
            break;
        }

        // reorient the line to originate from the collision point
        l.p = line_coord(l, t);

        l = reflect_line_object(&l, o);

        // flip the line to make it point _towards_ the next potential collision
        l.b = scalar_prod(-1, l.b);
    }

    if(n == N) {
        return black;
    }

    color_t c = black;
    for(ssize_t j = n; j >= 0; j--) {
        c = color_mix(cs[n].m.color, c);
        c = color_add(cs[n].m.light, c);
    }
    return c;
}

void rt_draw(color_t buf[], size_t height, size_t width)
{
    stopwatch_start(stopwatch);

    for(size_t i = 0; i < height; i++) {
        for(size_t j = 0; j < width; j++) {
            vec_t p = grid_coord(view.plane, (float)j - width/2, (float)i -height/2);
            line_t l = line_from_two_points(view.camera, p);

            trace("checking line: p=(%f,%f,%f) b=(%f,%f,%f)",
                  l.p.x, l.p.y, l.p.z,
                  l.b.x, l.b.y, l.b.z);

            color_t c = black;
            const object_t* o = find_collision(&l, &world, NULL);
            if(o) { c = o->material.color; }
            buf[i*width + j] = c;
        }
    }

    stopwatch_stop(stopwatch);
}
