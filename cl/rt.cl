/* vim: set ft=c: */

vec_t line_coord(line_t l, float t)
{
    return mad(t, l.b, l.p);
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

vec_t disperse(vec_t n, seed_t seed)
{
    vec_t d, e;
    if(dot(n, d = unit_vectors(rnd(&seed))) < 0) d *= -1;
    if(dot(n, e = unit_vectors(rnd(&seed))) < 0) e *= -1;
    return normalize(mix(d, e, uniform_float(seed)));
}

typedef struct {
    material_t m;
} ray_collision_t;

inline ray_collision_t sky_collision(__constant sky_t* s, const line_t* l)
{
    float f = max(1 - acos(dot(fast_normalize(s->sun), l->b))/M_PI_F, s->min);
    return (ray_collision_t) {
        .m = {
            .light = color(f*s->color.r, f*s->color.g, f*s->color.b),
            .color = black
        }
    };
}

vec_t object_normal(vec_t p, __constant object_t* o)
{
    switch(o->shape_type) {
    case SHAPE_TYPE_SPHERE:
        return (p - o->shape.sphere.c)/o->shape.sphere.r;
    case SHAPE_TYPE_PLANE:
        return o->shape.plane.n;
    }
}

// pre-conditions: l->p is in the surface of o->shape
line_t reflect_line_object(line_t* l, __constant object_t* o)
{
    const vec_t n = object_normal(l->p, o);
    return (line_t) { .p = l->p, .b = fma(2*dot(l->b, n), n, -l->b) };
}

#define RAY_TRACE_DEPTH 10

color_t ray_trace_one_line(__constant world_t* w, const line_t* line, seed_t s)
{
    ray_collision_t cs[RAY_TRACE_DEPTH];

    line_t l = *line; int o = -1;
    size_t n = 0; for(; n < RAY_TRACE_DEPTH; n++) {
        float t;
        o = find_collision(&l, w, &t, o);
        if(o < 0) {
            cs[n] = sky_collision(&w->sky, &l);
            break;
        } else {
            cs[n] = (ray_collision_t){ .m = w->objects[o].material };
        }

        // reorient the line to originate from the collision point
        l.p = line_coord(l, t);
        l.b = -l.b;

        l = reflect_line_object(&l, &w->objects[o]);

        if((rnd(&s) & w->objects[o].material.disperse) != 0) {
            l.b = disperse(object_normal(l.p, &w->objects[o]), s);
        }
    }

    if(n == RAY_TRACE_DEPTH) { return black; }

    color_t c = black;
    for(int j = n; j >= 0; j--) {
        c = color_mix(cs[j].m.color, c);
        c = color_add(cs[j].m.light, c);
    }
    return c;
}

/* pre-condigtion: exists k: Even, (N = get_global_size) == 1 + k^2 */
__kernel void rt_ray_trace(__constant world_t* world, __global color_t out[])
{
    const long y = get_global_id(0), H = get_global_size(0);
    const long x = get_global_id(1), W = get_global_size(1);
    const size_t n = get_global_id(2), N = get_global_size(2);

    seed_t s = world->seed;
    s += 134775813*x;
    s += 12345*y;
    s += 25214903917*n;

    const vec_t u = /* stage forward */ world->view.look_at - world->view.camera;
    const vec_t v = /* stage left */ normalize(cross(world->view.up, u));
    const vec_t w = /* stage up */ normalize(
        world->view.allow_tilt_shift ? world->view.up : cross(u, v)
    );

    const float a = (float)H/W;
    const float h = -2 * length(u) * tan(world->view.fov/2) / sqrt(1 + a*a);
    const vec_t b0 = h *     v / W;
    const vec_t b1 = h * a * w / H;
    vec_t p = world->view.look_at + (x - W/2)*b0 + (y - H/2)*b1;

    if(N > 1) {
        const float k = sqrt((float)(N-1));
        int quo, rem = remquo(n, k, &quo);
        p += ((quo - 2)*b0 + (rem - 2)*b1) / k;
    }

    const line_t l = line_from_two_points(world->view.camera, p);
    out[(y*W + x)*N + n] = ray_trace_one_line(world, &l, s);
}

__kernel void rt_sample(__constant color_t in[], const ulong N,
                        __global color_t out[])
{
    const long Y = get_global_id(0), H = get_global_size(0);
    const long X = get_global_id(1), W = get_global_size(1);

    color_t p = in[(Y*W + X)*N];

    uint3 c = 0; ulong M = 0; const long s = 0;
    for(long i = -s; i <= s; i++) {
        for(long j = -s; j <= s; j++) {
            long x = X + i, y = Y + j;
            if(x >= 0 && x < W && y >= 0 && y < H) {
                const ulong k = 1 + 2*s - (abs(i) + abs(j));
                for(size_t n = 0; n < N; n++) {
                    const color_t s = in[(y*W + x)*N + n];
                    M += k;
                    c.s0 += k*s.r; c.s1 += k*s.g; c.s2 += k*s.b;
                }
            }
        }
    }

    out[Y*W + X] = color(c.s0/M, c.s1/M, c.s2/M);
}
