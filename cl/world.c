#include <math.h>

world_t* create_world(float t, float duration, float fps)
{
    world_t* world = calloc(1, world_size_with_objects(5));
    world->objects_len = 5;

    world->seed = xorshift128plus_i();

    float angle = 2*2*M_PI/(duration*fps);
    world->view.camera = vec(10 - 20*cos(angle*t), 20*sin(angle*t), 10);

    world->view.up = vec(0, 0, 1);
    world->view.look_at = vec(10, 0, 5);
    world->view.fov = M_PI/2;

    world->sky.sun = vec(1, 1, 1);
    world->sky.min = 0.1;
    world->sky.color = color(0x40, 0x10, 0x80);

    world->objects[0] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(10, 1, 6), .r = 3 },
        .material = {
            .light = white,
            .color = black,
            .disperse = PROB_ALWAYS,
        },
    };

    world->objects[1] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_PLANE,
        .shape.plane = { .p = vec(0, 0, 0), .n = vec(0, 0, 1) },
        .material = {
            .light = black,
            .color = color(0x90, 0x70, 0x70),
            .disperse = PROB_ALWAYS
        },
    };

    world->objects[2] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(9, -7, 2), .r = 2 },
        .material = {
            .light = black,
            .color = color(0x50, 0x50, 0xff),
            .disperse = PROB_NEVER
        },
    };

    world->objects[3] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(8, 7, 3), .r = 2 },
        .material = {
            .light = black,
            .color = green,
            .disperse = PROB_ALWAYS
        },
    };

    world->objects[4] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(70, 40, 15), .r = 8 },
        .material = {
            .light = orange,
            .color = black,
            .disperse = PROB_NEVER
        },
    };

    return world;
}
