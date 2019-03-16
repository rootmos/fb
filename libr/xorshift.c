#include "r/xorshift.h"
#include "r/fail.h"

#include <fcntl.h>
#include <assert.h>
#include <unistd.h>

#ifndef ENTROPY_SOURCE
#define ENTROPY_SOURCE "/dev/urandom"
#endif

void random_bytes(void* buf, size_t n)
{
    int fd = open(ENTROPY_SOURCE, O_RDONLY);
    CHECK(fd, "open("ENTROPY_SOURCE")");
    ssize_t r = read(fd, buf, n); CHECK(r, "read("ENTROPY_SOURCE", %zu", n);
    assert(r == n);
    int s = close(fd); CHECK(s, "close("ENTROPY_SOURCE")");
}

uint64_t xorshift_state[2];
void xorshift_state_initalize(void)
{
    random_bytes(xorshift_state, sizeof(xorshift_state));
}
