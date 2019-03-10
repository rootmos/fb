#include <r.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>

const char* now_iso8601(void)
{
    static char buf[17];
    const time_t t = time(NULL);
    size_t r = strftime(buf, sizeof(buf), "%Y%m%dT%H%M%SZ", gmtime(&t));
    assert(r > 0);
    return buf;
}

const char* getenv_mandatory(const char* const env)
{
    const char* const v = getenv(env);
    if(v == NULL) { failwith("environment variable %s not set", env); }
    return v;
}
