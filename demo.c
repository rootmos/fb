#include "util.h"

#include <linux/fb.h>
#include <stropts.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

struct fb_fix_screeninfo fi;
char* fb;

static void clear()
{
    memset(fb, 0, fi.smem_len);
}

static void put_pixel(const size_t x, const size_t y, int c)
{
    const size_t off = x + y * fi.line_length;
    *(fb + off) = c;
}

static void draw_rect(const size_t x, const size_t y,
                      const size_t h, const size_t w,
                      int c)
{
    for(size_t i = 0; i < w; i++) {
        for(size_t j = 0; j < h; j++) {
            put_pixel(x + i, y + j, c);
        }
    }
}

int main(int argc, char* argv[])
{
    assert(argc == 2);
    const char* const fbfn = argv[1];
    const int fbfd = open(fbfn, O_RDWR); CHECK(fbfd, "open(%s, O_RDWR)", fbfn);

    struct fb_var_screeninfo orig_vi, vi;
    int r = ioctl(fbfd, FBIOGET_VSCREENINFO, &orig_vi);
    CHECK(r, "ioctl(FBIOGET_VSCREENINFO)");
    memcpy(&vi, &orig_vi, sizeof(vi));

    vi.bits_per_pixel = 8;
    r = ioctl(fbfd, FBIOPUT_VSCREENINFO, &vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    r = ioctl(fbfd, FBIOGET_FSCREENINFO, &fi);
    CHECK(r, "ioctl(FBIOGET_FSCREENINFO)");

    info("framebuffer: %" PRIu32 "x%" PRIu32", %" PRIu32 "bpp",
         vi.xres, vi.yres, vi.bits_per_pixel);

    fb = mmap(NULL, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    CHECK_NOT(fb, MAP_FAILED, "mmap(fbfd)");

    clear();
    for (int c = 0; c < 0x0a; c++) {
        draw_rect(10, 10, 300, 300, c);
        usleep(100000);
    }
    clear();

    r = ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    r = close(fbfd); CHECK(r, "close(fbfd)");
    r = munmap(fb, fi.smem_len); CHECK(r, "munmap");

    return 0;
}
