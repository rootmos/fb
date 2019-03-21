#include <CL/cl.h>
#include <r.h>
#include "rt.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static struct {
    cl_context ctx;
    cl_command_queue q;
    cl_program p;

    struct stopwatch* stopwatch_init;
    struct stopwatch* stopwatch_draw;
    struct stopwatch* stopwatch_write;

    entropy_t* entropy;
} rt_state;

void rt_write_ppm(int fd, const color_t buf[], size_t width, size_t height)
{
    stopwatch_start(rt_state.stopwatch_write);

    int r = dprintf(fd, "P6\n%zu %zu\n255\n", width, height);
    CHECK_IF(r < 0, "dprintf");

    size_t i = 0; const size_t N = sizeof(color_t) * height * width;
    while(i < N) {
        r = write(fd, buf + i, N - i);
        CHECK_IF(r < 0, "write");
        i += r;
    }

    r = close(fd); CHECK(r, "close");

    stopwatch_stop(rt_state.stopwatch_write);
}


void rt_error_callback(const char* err, const void* pi, size_t pi_len, void* data)
{
    error("%s", err);
}

void rt_build_callback(cl_program p, void* data)
{
    cl_int r; cl_uint ds;
    r = clGetProgramInfo(p, CL_PROGRAM_NUM_DEVICES, sizeof(ds), &ds, NULL);
    CHECK_OCL(r, "clGetProgramInfo");

    cl_device_id ids[ds];
    r = clGetProgramInfo(p, CL_PROGRAM_DEVICES, sizeof(ids), ids, NULL);
    CHECK_OCL(r, "clGetProgramInfo");

    for(size_t i = 0; i < ds; i++) {
        cl_build_status s;
        r = clGetProgramBuildInfo(
            p, ids[i], CL_PROGRAM_BUILD_STATUS, sizeof(s), &s, NULL);
        CHECK_OCL(r, "clGetProgramBuildInfo");

        if(s == CL_BUILD_ERROR) {
            char buf[16384];

            r = clGetProgramBuildInfo(
                p, ids[i], CL_PROGRAM_BUILD_LOG, sizeof(buf), buf, NULL);
            CHECK_OCL(r, "clGetProgramBuildInfo");

            error("build error: %s", buf);
        }
    }
}

entropy_t* rt_entropy(size_t uniform, size_t normal)
{
    entropy_t* entropy = calloc(sizeof(entropy_t), 1);

    entropy->uniform_N = uniform;
    entropy->uniform = calloc(sizeof(uint64_t), uniform);
    for(size_t i = 0; i < uniform; i++) {
        entropy->uniform[i] = xorshift128plus_i();
    }

    entropy->normal_N = normal;
    entropy->normal = calloc(sizeof(float), normal);
    for(size_t i = 0; i < normal; i++) {
        entropy->normal[i] = normal_dist_i();
    }

    return entropy;
}

void rt_initialize(void)
{
    rt_state.stopwatch_init = stopwatch_mk("rt_initialize", 1);
    rt_state.stopwatch_draw = stopwatch_mk("rt_draw", 1);
    rt_state.stopwatch_write = stopwatch_mk("rt_write", 1);

    stopwatch_start(rt_state.stopwatch_init);

    xorshift_state_initalize();
    rt_state.entropy = rt_entropy(128, 128);

    const char* src[] = { R"(
#include "types.cl"
)", R"(
#include "rnd.cl"
)", R"(
#include "shared.h"
)", R"(
#include "rt.cl"
)", };

    size_t src_len[LENGTH(src)];
    for(size_t i = 0; i < LENGTH(src_len); i++) {
        src_len[i] = strlen(src[i]);
    }

    cl_uint ds;
    cl_int r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 0, NULL, &ds);
    CHECK_OCL(r, "clGetDeviceIDs");

    cl_device_id ids[ds];
    r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, ds, ids, NULL);
    CHECK_OCL(r, "clGetDeviceIDs");

    cl_device_id def;
    r = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_DEFAULT, 1, &def, NULL);
    CHECK_OCL(r, "clGetDeviceIDs");

    char def_name[100];
    r = clGetDeviceInfo(def, CL_DEVICE_NAME, sizeof(def_name), def_name, NULL);
    CHECK_OCL(r, "clGetDeviceInfo");

    info("found %u device with default: %s", ds, def_name);

    rt_state.ctx = clCreateContext(
        NULL,
        ds, ids,
        rt_error_callback, NULL,
        &r);
    CHECK_OCL(r, "clCreateContext");

    rt_state.q = clCreateCommandQueueWithProperties(
        rt_state.ctx, def, NULL, &r);
    CHECK_OCL(r, "clCreateCommandQueueWithProperties");

    rt_state.p = clCreateProgramWithSource(
        rt_state.ctx,
        LENGTH(src), src, src_len, &r);
    CHECK_OCL(r, "clCreateProgramWithSource");

#ifndef DEBUG
    const char* flags = "-cl-std=CL2.0";
#else
    const char* flags = "-cl-std=CL2.0 -DDEBUG";
#endif

    r = clBuildProgram(rt_state.p, ds, ids, flags, rt_build_callback, NULL);
    CHECK_OCL(r, "clBuildProgram");

    stopwatch_stop(rt_state.stopwatch_init);
}

void rt_run(void)
{
    cl_int r = clFinish(rt_state.q); CHECK_OCL(r, "clFinish");
}

void rt_draw(const world_t* w, size_t width, size_t height, size_t samples,
             color_t buf[])
{
    stopwatch_start(rt_state.stopwatch_draw);
    cl_int r;

    // buffers
    cl_mem world = clCreateBuffer(rt_state.ctx,
        CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR,
        world_size(w), (void*)w, &r);
    CHECK_OCL(r, "world = clCreateBuffer");

    cl_mem uniform = clCreateBuffer(rt_state.ctx,
        CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR,
        rt_state.entropy->uniform_N * sizeof(uint64_t),
        rt_state.entropy->uniform, &r);
    CHECK_OCL(r, "uniform = clCreateBuffer");

    cl_mem normal = clCreateBuffer(rt_state.ctx,
        CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR,
        rt_state.entropy->normal_N * sizeof(float),
        rt_state.entropy->normal, &r);
    CHECK_OCL(r, "normal = clCreateBuffer");

    const size_t N = sizeof(color_t)*width*height;

    cl_mem data = clCreateBuffer(rt_state.ctx,
        CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS,
        N * samples, NULL, &r);
    CHECK_OCL(r, "out = clCreateBuffer");

    cl_mem out = clCreateBuffer(rt_state.ctx,
        CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
        N, NULL, &r);
    CHECK_OCL(r, "out = clCreateBuffer");


    // kernels
    cl_kernel rt = clCreateKernel(rt_state.p, "rt_ray_trace", &r);
    CHECK_OCL(r, "clCreateKernel");

    r = clSetKernelArg(rt, 0, sizeof(world), &world);
    CHECK_OCL(r, "clSetKernelArg");

    r = clSetKernelArg(rt, 1, sizeof(uniform), &uniform);
    CHECK_OCL(r, "clSetKernelArg");

    r = clSetKernelArg(rt, 2, sizeof(size_t), &rt_state.entropy->uniform_N);
    CHECK_OCL(r, "clSetKernelArg");

    r = clSetKernelArg(rt, 3, sizeof(normal), &normal);
    CHECK_OCL(r, "clSetKernelArg");

    r = clSetKernelArg(rt, 4, sizeof(size_t), &rt_state.entropy->normal_N);
    CHECK_OCL(r, "clSetKernelArg");

    r = clSetKernelArg(rt, 5, sizeof(data), &data);
    CHECK_OCL(r, "clSetKernelArg");


    cl_kernel sampler = clCreateKernel(rt_state.p, "rt_sample", &r);
    CHECK_OCL(r, "clCreateKernel");

    r = clSetKernelArg(sampler, 0, sizeof(data), &data);
    CHECK_OCL(r, "clSetKernelArg");

    r = clSetKernelArg(sampler, 1, sizeof(samples), &samples);
    CHECK_OCL(r, "clSetKernelArg");

    r = clSetKernelArg(sampler, 2, sizeof(out), &out);
    CHECK_OCL(r, "clSetKernelArg");

    // enqueue
    cl_event e0;
    r = clEnqueueNDRangeKernel(
        rt_state.q, rt, 3, NULL, (size_t[]){ height, width, samples }, NULL,
        0, NULL, &e0);
    CHECK_OCL(r, "clEnqueueNDRangeKernel");

    cl_event e1;
    r = clEnqueueNDRangeKernel(
        rt_state.q, sampler, 2, NULL, (size_t[]){ height, width }, NULL,
        1, (cl_event[]){ e0 }, &e1);
    CHECK_OCL(r, "clEnqueueNDRangeKernel");

    r = clEnqueueReadBuffer(
        rt_state.q, out, CL_TRUE, 0, N, buf,
        1, (cl_event[]){ e1 }, NULL);
    CHECK_OCL(r, "clEnqueueReadBuffer");

    stopwatch_stop(rt_state.stopwatch_draw);
}
