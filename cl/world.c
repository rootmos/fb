#include <stdlib.h>
#include <math.h>

const world_t* create_world(void)
{
    world_t* world = calloc(1, world_size_with_objects(5));
    world->objects_len = 5;

    world->seed = xorshift128plus_i();

    world->view.camera = vec(-10.0, 0, 10);
    world->view.up = vec(0, 0, 1);
    world->view.look_at = vec(0, 0, 5);
    world->view.fov = M_PI/2;

    world->objects[0] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(10, 1, 6), .r = 3 },
        .material = {
            .light = white,
            .color = black,
            .dispersion = 0.3
        },
    };

    world->objects[1] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_PLANE,
        .shape.plane = { .p = vec(0, 0, 0), .n = vec(0, 0, 1) },
        .material = {
            .light = black,
            .color = color(0x90, 0x70, 0x70),
            .dispersion = 0.03
        },
    };

    world->objects[2] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(9, -7, 2), .r = 2 },
        .material = {
            .light = black,
            .color = color(0x50, 0x50, 0xff),
            .dispersion = 0.01
        },
    };

    world->objects[3] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(8, 7, 3), .r = 2 },
        .material = {
            .light = black,
            .color = green,
            .dispersion = 0.4
        },
    };

    world->objects[4] = (object_t) {
        .unique.seed = xorshift128plus_i(),
        .shape_type = SHAPE_TYPE_SPHERE,
        .shape.sphere = { .c = vec(70, 40, 30), .r = 8 },
        .material = {
            .light = orange,
            .color = black,
            .dispersion = 0.001
        },
    };

    return world;
}
