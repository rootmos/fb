#include <r.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const char* now_iso8601(void)
{
    static char buf[17];
    const time_t t = time(NULL);
    size_t r = strftime(buf, sizeof(buf), "%Y%m%dT%H%M%SZ", gmtime(&t));
    assert(r > 0);
    return buf;
}

static void log_prefix(const char* const caller,
                       const char* const file,
                       const unsigned int line)
{
    fprintf(stderr, "%s:%d:%s:%s:%u ",
            now_iso8601(), getpid(), caller, file, line);
}

void __info(const char* const caller,
            const char* const file,
            const unsigned int line,
            const char* const fmt, ...)
{
    log_prefix(caller, file, line);

    va_list vl;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);
}

void __failwith(const char* const caller,
                const char* const file,
                const unsigned int line,
                const int include_errno,
                const char* const fmt, ...)
{
    log_prefix(caller, file, line);

    if(include_errno) {
        fprintf(stderr, "(%s) ", strerror(errno));
    }

    va_list vl;
    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);

    abort();
}


const char* getenv_mandatory(const char* const env)
{
    const char* const v = getenv(env);
    if(v == NULL) { failwith("environment variable %s not set", env); }
    return v;
}
