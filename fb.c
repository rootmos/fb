#include <r.h>
#include "fb.h"

#include <string.h>
#include <linux/fb.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <stropts.h>

static struct {
    size_t frame;

    struct fb_fix_screeninfo fi;
    struct fb_var_screeninfo vi;
    struct fb_var_screeninfo orig_vi;
    int fbfd;
    char* fb;
    size_t page_size;
    uint_fast8_t cur_page;
} ctx;

void fb_clear(void)
{
    memset(ctx.fb + ctx.page_size * ctx.cur_page, 0, ctx.page_size);
}

void fb_open(const char* const fbfn)
{
    ctx.fbfd = open(fbfn, O_RDWR); CHECK(ctx.fbfd, "open(%s, O_RDWR)", fbfn);

    int r = ioctl(ctx.fbfd, FBIOGET_VSCREENINFO, &ctx.orig_vi);
    CHECK(r, "ioctl(FBIOGET_VSCREENINFO)");
    memcpy(&ctx.vi, &ctx.orig_vi, sizeof(ctx.vi));

    ctx.vi.bits_per_pixel = 24;
    ctx.vi.xres_virtual = ctx.vi.xres;
    ctx.vi.yres_virtual = ctx.vi.yres * 2;
    r = ioctl(ctx.fbfd, FBIOPUT_VSCREENINFO, &ctx.vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    info("framebuffer: %" PRIu32 "x%" PRIu32", %" PRIu32 "bpp",
         ctx.vi.xres, ctx.vi.yres, ctx.vi.bits_per_pixel);

    r = ioctl(ctx.fbfd, FBIOGET_FSCREENINFO, &ctx.fi);
    CHECK(r, "ioctl(FBIOGET_FSCREENINFO)");

    ctx.fb = mmap(NULL, ctx.fi.smem_len,
                  PROT_READ | PROT_WRITE, MAP_SHARED, ctx.fbfd, 0);
    CHECK_NOT(ctx.fb, MAP_FAILED, "mmap(fbfd)");

    ctx.page_size = ctx.fi.line_length * ctx.vi.yres;
    memset(ctx.fb, 0, ctx.fi.smem_len);
}

void fb_close(void)
{
    int r = ioctl(ctx.fbfd, FBIOPUT_VSCREENINFO, &ctx.orig_vi);
    CHECK(r, "ioctl(FBIOPUT_VSCREENINFO)");

    r = close(ctx.fbfd); CHECK(r, "close(fbfd)");
    r = munmap(ctx.fb, ctx.fi.smem_len); CHECK(r, "munmap");
}

void fb_flip(void)
{
    ctx.vi.yoffset = ctx.cur_page * ctx.vi.yres;
    ctx.vi.activate = FB_ACTIVATE_VBL;
    int r = ioctl(ctx.fbfd, FBIOPAN_DISPLAY, &ctx.vi);
    CHECK(r, "ioctl(FBIOPAN_DISPLAY)");
    r = ioctl(ctx.fbfd, FBIO_WAITFORVSYNC, NULL);
    CHECK(r, "ioctl(FBIO_WAITFORVSYNC)");

    ctx.cur_page = (ctx.cur_page + 1) % 2;
    ctx.frame += 1;
}

inline void fb_pixel(const size_t x, const size_t y, const color_t c)
{
    const size_t off =
        (x * 3) + y * ctx.fi.line_length + ctx.cur_page * ctx.page_size;
    *(color_t*)(ctx.fb + off) = c;
}

void fb_rect(const size_t x, const size_t y, const size_t h, const size_t w,
             const color_t c)
{
    for(size_t i = 0; i < w; i++) {
        for(size_t j = 0; j < h; j++) {
            fb_pixel(x + i, y + j, c);
        }
    }
}
