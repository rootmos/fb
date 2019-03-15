#pragma once

#define LENGTH(xs) (sizeof(xs)/sizeof((xs)[0]))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

const char* getenv_mandatory(const char* const env);

// returns current time formated as compact ISO8601: 20190123T182628Z
const char* now_iso8601(void);
