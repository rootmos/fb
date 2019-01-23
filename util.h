#pragma once

#define CHECK(res, format, ...) CHECK_NOT(res, -1, format, ##__VA_ARGS__)

#define CHECK_NOT(res, err, format, ...) \
    CHECK_IF(res == err, format, ##__VA_ARGS__)

#define CHECK_IF(cond, format, ...) do { \
    if(cond) { \
        __failwith(__extension__ __FUNCTION__, __extension__ __FILE__, \
                   __extension__ __LINE__, 1, \
                   format "\n", ##__VA_ARGS__); \
    } \
} while(0)

#define info(format, ...) do { \
    __info(__extension__ __FUNCTION__, __extension__ __FILE__, \
           __extension__ __LINE__, format "\n", ##__VA_ARGS__); \
} while(0)

#define failwith(format, ...) \
    __failwith(__extension__ __FUNCTION__, __extension__ __FILE__, \
               __extension__ __LINE__, 0, format "\n", ##__VA_ARGS__)

#define not_implemented() failwith("not implemented")

void __failwith(const char* const caller,
                const char* const file,
                const unsigned int line,
                const int include_errno,
                const char* const fmt, ...)
    __attribute__ ((noreturn, format (printf, 5, 6)));

void __info(const char* const caller,
            const char* const file,
            const unsigned int line,
            const char* const fmt, ...)
    __attribute__ ((format (printf, 4, 5)));

const char* getenv_mandatory(const char* const env);

// returns current time formated as: 20190123T182628Z
const char* now_iso8601(void);
