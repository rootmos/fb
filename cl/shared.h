#define color(rr,gg,bb) ((color_t){ .r = rr, .g = gg, .b = bb })

#define black color(0, 0, 0)
#define white color(0xff, 0xff, 0xff)
#define red color(0xff, 0x00, 0x00)
#define green color(0x00, 0xff, 0x00)
#define blue color(0x00, 0x00, 0xff)
#define violet color(0x7f, 0x00, 0xff)
#define orange color(0xff, 0x80, 0x00)

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

typedef struct {
    color_t color;
    color_t light;
    float dispersion;
} material_t;

// |v - c| = r
typedef struct { vec_t c; float r; } sphere_t;

// p + span(b_0, b_1)
typedef struct {
    vec_t p;
    vec_t b[2];
} grid_t;

typedef enum {
    SHAPE_TYPE_SPHERE,
    SHAPE_TYPE_PLANE,
} shape_type_t;

typedef struct {
    union {
        seed_t seed;
        unsigned int id;
    } unique;

    shape_type_t shape_type;
    union {
        sphere_t sphere;
        plane_t plane;
    } shape;
    material_t material;
} object_t;

typedef struct {
    vec_t camera;
    grid_t plane;
} viewport_t;


typedef struct {
    seed_t seed;
    viewport_t view;
    size_t objects_len;
    object_t objects[];
} world_t;

static inline size_t world_size(const world_t* w)
{
    return sizeof(world_t) + sizeof(object_t)*w->objects_len;
}
