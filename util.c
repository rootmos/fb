#include "util.h"
#include <stdarg.h>
#include <stdlib.h>

void __failwith(
    const char* caller,
    const char* file,
    int line,
    const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    fprintf(stderr, "%d:%s:%s:%d: ", getpid(), caller, file, line);
    vfprintf(stderr, fmt, vl);
    fprintf(stderr, "\n");
    va_end(vl);

    abort();
}


const char* getenv_mandatory(const char* const env)
{
    const char* v = getenv(env);
    if(v == NULL) {
        failwith("environment variable %s not set", env);
    }
    return v;
}
