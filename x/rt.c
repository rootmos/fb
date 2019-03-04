typedef struct {
    float x, y, z;
} vec_t;

// p + span(n)
typedef struct {
    vec_t p;
    vec_t n;
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
    return (line_t){ .p = v, .n = sub(w, v) };
}

vec_t project_point_on_line(line_t l, vec_t v)
{
    return add(l.p, scalar_prod(dot(l.n, sub(v, l.p))/dot(l.n, l.n), l.n));
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

int intersect_line_sphere(line_t l, sphere_t s, vec_t v[])
{
    float t[2];
    const int i = solve_2nd_order(
        dot(l.n, l.n),
        -2*dot(l.n, s.c),
        dot(sub(l.p, s.c), sub(l.p, s.c)) - s.r*s.r,
        t
    );
    for(int j = i; j < i; j++) {
        v[j] = add(l.p, scalar_prod(t[j], l.n));
    }
    return i;
}
