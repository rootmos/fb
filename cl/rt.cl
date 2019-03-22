/* vim: set ft=c: */

vec_t line_coord(line_t l, float t)
{
    return mad(t, l.b, l.p);
}

vec_t grid_coord(grid_t g, float s, float t)
{
    return mad(s, g.b[0], mad(t, g.b[1], g.p));
}

line_t line_from_two_points(vec_t v, vec_t w)
{
    return (line_t){ .p = v, .b = normalize(w - v) };
}

inline color_t color_mix(color_t factor, color_t c)
{
    return color(c.r*factor.r/0xff, c.g*factor.g/0xff, c.b*factor.b/0xff);
}

inline color_t color_add(color_t x, color_t y)
{
    return color(min(x.r+y.r, 0xff), min(x.g+y.g, 0xff), min(x.b+y.b, 0xff));
}

inline bool is_zero(float x)
{
    return fast_length(x) < 1e-12;
}

// t^2 + 2bt + c = 0
int solve_2nd_order(float b, float c, float t[])
{
    float d = b*b - c;
    if(d < 0) return 0;

    const float p = -b;
    d = sqrt(d);
    t[0] = p + d;
    t[1] = p - d;
    return 2;
}

inline int intersect_line_sphere(line_t* l, __constant sphere_t* s, float t[])
{
    const vec_t d = l->p - s->c;
    return solve_2nd_order(
        dot(l->b, d),
        dot(d, d) - s->r*s->r,
        t
    );
}

int intersect_line_plane(line_t* l, __constant plane_t* p, float t[])
{
    const float u = dot(p->p - l->p, p->n);
    if(is_zero(u)) return t[0] = 0, 1; // line is in the plane
    const float v = dot(l->b, p->n);
    if(is_zero(v)) return 0; // line is parallel
    return t[0] = u / v, 1;
}

int intersect_line_object(line_t* l, __constant object_t* o, float t[])
{
    switch(o->shape_type) {
    case SHAPE_TYPE_SPHERE:
        return intersect_line_sphere(l, &o->shape.sphere, t);
    case SHAPE_TYPE_PLANE:
        return intersect_line_plane(l, &o->shape.plane, t);
    }
    return -1;
}

int find_collision(line_t* l, __constant world_t* w, float* t, int exclude)
{
    float t_min = -1; size_t n = w->objects_len;

    for(size_t i = 0; i < w->objects_len; i++) {
        if(i == exclude) continue;

        float s[2];
        int r = intersect_line_object(l, &w->objects[i], s);

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
        return n;
    } else {
        return -1;
    }
}

line_t disperse(line_t l, float factor, seed_t seed)
{
    seed = rnd_combine((seed_t[]){ seed, seed_from_vec(l.p), seed_from_vec(l.b) }, 3);

    vec_t d = factor * (float)M_PI * vec(
        normal_dist(&seed),
        normal_dist(&seed),
        normal_dist(&seed)
    );

    return (line_t){
        .p = l.p,
        .b = normalize(l.b*cos(d) - sqrt(1 - l.b*l.b)*sin(d))
    };
}

typedef struct {
    material_t m;
} ray_collision_t;

inline ray_collision_t sky_collision(const line_t* l)
{
    return (ray_collision_t) {
        .m = { .light = color(0x40, 0x10, 0x80), .color = black }
    };
}

// pre-conditions: l->p is in the surface of o->shape
line_t reflect_line_object(line_t* l, __constant object_t* o)
{
    vec_t n;
    switch(o->shape_type) {
    case SHAPE_TYPE_SPHERE:
        n = (l->p - o->shape.sphere.c)/o->shape.sphere.r;
        break;
    case SHAPE_TYPE_PLANE:
        n = o->shape.plane.n;
        break;
    }

    return (line_t) {
        .p = l->p,
        .b = fma(2*dot(l->b, n), n, -l->b)
    };
}

#define RAY_TRACE_DEPTH 10

color_t ray_trace_one_line(__constant world_t* w, const line_t* line)
{
    ray_collision_t cs[RAY_TRACE_DEPTH];

    line_t l = *line; int o = -1;
    size_t n = 0; for(; n < RAY_TRACE_DEPTH; n++) {
        float t;
        o = find_collision(&l, w, &t, o);
        if(o < 0) {
            cs[n] = sky_collision(&l);
            break;
        } else {
            cs[n] = (ray_collision_t){ .m = w->objects[o].material };
        }

        // reorient the line to originate from the collision point
        l.p = line_coord(l, t);
        l.b = -l.b;

        l = reflect_line_object(&l, &w->objects[o]);

        l = disperse(
            l,
            w->objects[o].material.dispersion,
            w->objects[o].unique.seed
        );
    }

    if(n == RAY_TRACE_DEPTH) { return black; }

    color_t c = black;
    for(int j = n; j >= 0; j--) {
        c = color_mix(cs[j].m.color, c);
        c = color_add(cs[j].m.light, c);
    }
    return c;
}

__kernel void rt_ray_trace(__constant world_t* world, __global color_t out[])
{
    const size_t y = get_global_id(0), h = get_global_size(0);
    const size_t x = get_global_id(1), w = get_global_size(1);
    const size_t n = get_global_id(2), N = get_global_size(2);

    vec_t p = grid_coord(world->view.plane, (float)x - w/2, (float)y - h/2);
    line_t l = line_from_two_points(world->view.camera, p);
    l = disperse(l, 0.0002, world->seed * (n + 1));
    out[(y*w + x)*N + n] = ray_trace_one_line(world, &l);
}

__kernel void rt_sample(__constant color_t in[], ulong N, __global color_t out[])
{
    const size_t y = get_global_id(0);
    const size_t x = get_global_id(1), w = get_global_size(1);
    uint3 c = 0;
    __constant color_t* samples = &in[(y*w + x)*N];
    for(size_t n = 0; n < N; n++) {
        const color_t s = samples[n];
        c.s0 += s.r; c.s1 += s.g; c.s2 += s.b;
    }
    out[y*w + x] = color(c.s0/N, c.s1/N, c.s2/N);
}
