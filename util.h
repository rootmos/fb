#pragma once

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define CHECK(res, format, ...) CHECK_NOT(res, -1, format, ##__VA_ARGS__)

#define CHECK_NOT(res, err, format, ...) CHECK_IF(res == err, format, ##__VA_ARGS__)

#define CHECK_IF(cond, format, ...) do { \
    if(cond) { \
        fprintf(stderr, "%d:%s():%s:%d: (%s) " format "\n", \
                getpid(), \
                __extension__ __FUNCTION__,  __extension__ __FILE__, __extension__ __LINE__, \
                strerror(errno), ##__VA_ARGS__); \
        exit(3); \
    } \
} while(0)

#define info(format, ...) do { \
    fprintf(stderr, "%d:%s():%s:%d: " format "\n", getpid(), __extension__ __FUNCTION__,  __extension__ __FILE__, __extension__ __LINE__, ##__VA_ARGS__); \
} while(0)

#define failwith(...) __failwith(__extension__ __FUNCTION__, __extension__ __FILE__, __extension__ __LINE__, __VA_ARGS__)

void __failwith(const char* caller,
                const char* file,
                int line,
                const char* fmt, ...)
    __attribute__ ((noreturn));

const char* getenv_mandatory(const char* const env);
