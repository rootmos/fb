#include <stdlib.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include <r.h>

static xcb_screen_t* fetch_screen(xcb_connection_t* con, int screen)
{
    xcb_screen_iterator_t i = xcb_setup_roots_iterator(xcb_get_setup(con));
    int s = screen;
    for(; i.rem; --s, xcb_screen_next(&i)) {
        if(s == 0) return i.data;
    }

    failwith("can't find screen: %d", screen);
}

static void dump_screen_info(xcb_screen_t* s)
{
    info("pixels=%dx%d", s->width_in_pixels, s->height_in_pixels);
}

int main(int argc, char** argv)
{
    int s;
    xcb_connection_t* con = xcb_connect(NULL, &s);
    int r = xcb_connection_has_error(con);
    if(r != 0) { failwith("xcb_connection_has_error(...) == %d", r); }

    xcb_screen_t* sc = fetch_screen(con, s);
    dump_screen_info(sc);

    xcb_window_t wi = xcb_generate_id(con);

    xcb_void_cookie_t c = xcb_create_window_checked(
        con, XCB_COPY_FROM_PARENT,
        wi, sc->root,
        /* x, y */ 0, 0,
        /* w, h */ 150, 150,
        /* border */ 1,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        sc->root_visual,
        XCB_CW_EVENT_MASK,
        &(int[]){ XCB_EVENT_MASK_KEY_PRESS });
    xcb_generic_error_t* err = xcb_request_check(con, c);
    if(err) {
        failwith("xcb_create_window_checked failed: %u", err->error_code);
    }

    c = xcb_map_window_checked(con, wi);
    if((err = xcb_request_check(con, c))) {
        failwith("xcb_map_window_checked failed: %u", err->error_code);
    }

    xcb_gcontext_t gc = xcb_generate_id(con);
    c = xcb_create_gc_checked(con, gc, sc->root, 0, NULL);
    if((err = xcb_request_check(con, c))) {
        failwith("xcb_create_gc_checked failed: %u", err->error_code);
    }

    xcb_key_symbols_t* syms = xcb_key_symbols_alloc(con);

    if((r = xcb_flush(con)) <= 0) { failwith("xcb_flush(...) == %d", r); }

    xcb_generic_event_t* e; int bail = 0;
    while(!bail && (e = xcb_wait_for_event(con))) {
        switch(e->response_type & ~0x80) {
        case XCB_KEY_PRESS: {
            xcb_key_press_event_t* ev = (xcb_key_press_event_t*)e;
            xcb_keysym_t sym = xcb_key_symbols_get_keysym(syms, ev->detail, 0);
            switch(sym) {
            case XK_q:
            case XK_Q:
                bail = 1;
                break;
            default:
                info("unmapped key press: %x", sym);
            }
            break;
        }
        default:
            failwith("unexpected event: response_type=%u", e->response_type);
        }

        free(e);
    }

    xcb_key_symbols_free(syms);
    xcb_disconnect(con);
    return 0;
}
