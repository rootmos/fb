#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include <r.h>
#include "rt.h"

static xcb_screen_t* fetch_screen(xcb_connection_t* con, int screen)
{
    xcb_screen_iterator_t i = xcb_setup_roots_iterator(xcb_get_setup(con));
    int s = screen;
    for(; i.rem; --s, xcb_screen_next(&i)) {
        if(s == 0) return i.data;
    }

    failwith("can't find screen: %d", screen);
}

static void dump_screen_info(xcb_screen_t* s, int n)
{
    info("screen %d: pixels=%dx%d %dbpp", n,
         s->width_in_pixels, s->height_in_pixels,
         s->root_depth);
}

struct {
    int16_t x, y;
    uint16_t w, h;

    xcb_connection_t* con;
    xcb_window_t wi;
    xcb_gcontext_t gc;
    xcb_screen_t* sc;
    xcb_key_symbols_t* syms;
} state;

void x11_init(void)
{
    int s;
    state.con = xcb_connect(NULL, &s);
    int r = xcb_connection_has_error(state.con);
    if(r != 0) { failwith("xcb_connection_has_error(...) == %d", r); }

    state.sc = fetch_screen(state.con, s);
    dump_screen_info(state.sc, s);

    // create window
    state.wi = xcb_generate_id(state.con);
    state.x = 0; state.y = 0;
    state.w = 100; state.h = 100;
    xcb_void_cookie_t c = xcb_create_window_checked(
        state.con, XCB_COPY_FROM_PARENT,
        state.wi, state.sc->root,
        state.x, state.y,
        state.w, state.h,
        /* border */ 0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        state.sc->root_visual,
        XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        &(int[]){
            0x0000ff,
            XCB_EVENT_MASK_KEY_PRESS
                | XCB_EVENT_MASK_STRUCTURE_NOTIFY
                | XCB_EVENT_MASK_EXPOSURE
        });
    xcb_generic_error_t* err = xcb_request_check(state.con, c);
    if(err) {
        failwith("xcb_create_window_checked failed: %u", err->error_code);
    }

    // gc
    state.gc = xcb_generate_id(state.con);
    c = xcb_create_gc_checked(
        state.con, state.gc, state.sc->root,
        XCB_GC_BACKGROUND | XCB_GC_GRAPHICS_EXPOSURES, &(int[]){ 0x00aaee,  1 });
    if((err = xcb_request_check(state.con, c))) {
        failwith("xcb_create_gc_checked failed: %u", err->error_code);
    }

    // syms
    state.syms = xcb_key_symbols_alloc(state.con);

    // map window
    c = xcb_map_window_checked(state.con, state.wi);
    if((err = xcb_request_check(state.con, c))) {
        failwith("xcb_map_window_checked failed: %u", err->error_code);
    }
}

void x11_deinit(void)
{
    xcb_key_symbols_free(state.syms);
    xcb_disconnect(state.con);
}

void render(void)
{
    uint32_t buf[state.w * state.h];
    rt_draw((color_t*)buf, state.h, state.w);

    xcb_void_cookie_t c = xcb_put_image_checked(
        state.con, XCB_IMAGE_FORMAT_Z_PIXMAP,
        state.wi, state.gc,
        state.w, state.h,
        /* x */ 0, /* y */ 0,
        0, state.sc->root_depth,
        sizeof(buf), (uint8_t*)buf);
    xcb_generic_error_t* err = xcb_request_check(state.con, c);
    if(err) {
        failwith("xcb_put_image_checked failed: %u", err->error_code);
    }
}


void flush(void)
{
    int r = xcb_flush(state.con);
    if(r <= 0) { failwith("xcb_flush(...) == %d", r); }
}

void run_event_loop(void)
{
    // event loop
    xcb_generic_event_t* e; int bail = 0;
    while(!bail && (e = xcb_wait_for_event(state.con))) {
        switch(e->response_type & ~0x80) {
        case XCB_KEY_PRESS: {
            xcb_key_press_event_t* ev = (xcb_key_press_event_t*)e;
            xcb_keysym_t sym = xcb_key_symbols_get_keysym(state.syms,
                                                          ev->detail, 0);
            switch(sym) {
            case XK_q:
            case XK_Q:
                bail = 1;
                break;
            default:
                debug("unmapped key press: %x", sym);
            }
            break;
        }
        case XCB_CONFIGURE_NOTIFY: {
            xcb_configure_notify_event_t* ev = (xcb_configure_notify_event_t*)e;
            state.w = ev->width; state.h = ev->height;
            state.x = ev->x; state.y = ev->y;
            info("geometry: %" PRIi16 "x%" PRIi16 "+%" PRIu16 "+%" PRIu16,
                 state.w, state.h, state.x, state.y);

            render();

            break;
        }
        case XCB_EXPOSE: {
            xcb_expose_event_t* ev = (xcb_expose_event_t*)e;
            debug("expose (count=%" PRIu16 "): %" PRIu16 "x%" PRIu16
                 "+%" PRIu16 "+%" PRIu16,
                 ev->count, ev->width, ev->height, ev->x, ev->y);
            render();
        }
        case XCB_MAP_NOTIFY: {
            xcb_map_notify_event_t* ev = (xcb_map_notify_event_t*)e;
            debug("mapped: override_redirect=%" PRIu8,
                  ev->override_redirect);
            break;
        }
        case XCB_UNMAP_NOTIFY: {
            xcb_unmap_notify_event_t* ev = (xcb_unmap_notify_event_t*)e;
            debug("unmapped: from_configure=%" PRIu8, ev->from_configure);
            break;
        }
        default:
            failwith("unexpected event: response_type=%u", e->response_type);
        }

        free(e);
    }
}

int main(int argc, char** argv)
{
    rt_setup();
    x11_init();
    run_event_loop();
    x11_deinit();
    return 0;
}
