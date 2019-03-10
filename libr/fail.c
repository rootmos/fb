#include "r/util.h"
#include "r/logging.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

void r_failwith(const char* const caller,
                const char* const file,
                const unsigned int line,
                const int include_errno,
                const char* const fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);

    if(include_errno) {
        r_log(LOG_ERROR, caller, file, line, "(%s) ", strerror(errno));
        vfprintf(stderr, fmt, vl);
    } else {
        r_vlog(LOG_ERROR, caller, file, line, fmt, vl);
    }
    va_end(vl);

    abort();
}
