#include "gfx_x11.h"
#include "world_data.h"
#include <GL/gl.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifndef XA_STRING
#define XA_STRING 31
#endif
#ifndef XA_PRIMARY
#define XA_PRIMARY 1
#endif
#include <X11/cursorfont.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ------------------------------------------------------------------------- */
/* 5x7 bitmap font                                                           */
/* ------------------------------------------------------------------------- */

typedef struct {
    char ch;
    uint8_t rows[7];
} Glyph5x7;

static const Glyph5x7 FONT5X7[] = {
    { ' ',  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { '!',  { 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04 } },
    { '"',  { 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { '#',  { 0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A } },
    { '$',  { 0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04 } },
    { '%',  { 0x19, 0x1A, 0x02, 0x04, 0x08, 0x0B, 0x13 } },
    { '&',  { 0x0C, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0D } },
    { '\'', { 0x04, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00 } },
    { '(',  { 0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02 } },
    { ')',  { 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08 } },
    { '*',  { 0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00 } },
    { '+',  { 0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00 } },
    { ',',  { 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08 } },
    { '-',  { 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 } },
    { '.',  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C } },
    { '/',  { 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10 } },
    { '0',  { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E } },
    { '1',  { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { '2',  { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F } },
    { '3',  { 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E } },
    { '4',  { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 } },
    { '5',  { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E } },
    { '6',  { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E } },
    { '7',  { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 } },
    { '8',  { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E } },
    { '9',  { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C } },
    { ':',  { 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00 } },
    { ';',  { 0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x08 } },
    { '<',  { 0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02 } },
    { '=',  { 0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00 } },
    { '>',  { 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08 } },
    { '?',  { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04 } },
    { '@',  { 0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0E } },
    { 'A',  { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'B',  { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E } },
    { 'C',  { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E } },
    { 'D',  { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E } },
    { 'E',  { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F } },
    { 'F',  { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 } },
    { 'G',  { 0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F } },
    { 'H',  { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 } },
    { 'I',  { 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E } },
    { 'J',  { 0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C } },
    { 'K',  { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 } },
    { 'L',  { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F } },
    { 'M',  { 0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11 } },
    { 'N',  { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 } },
    { 'O',  { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'P',  { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 } },
    { 'Q',  { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D } },
    { 'R',  { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 } },
    { 'S',  { 0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E } },
    { 'T',  { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { 'U',  { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E } },
    { 'V',  { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 } },
    { 'W',  { 0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A } },
    { 'X',  { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 } },
    { 'Y',  { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 } },
    { 'Z',  { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F } },
    { '[',  { 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E } },
    { '\\', { 0x10, 0x08, 0x08, 0x04, 0x02, 0x02, 0x01 } },
    { ']',  { 0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E } },
    { '^',  { 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00 } },
    { '_',  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F } },
    { '`',  { 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { '{',  { 0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02 } },
    { '|',  { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 } },
    { '}',  { 0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08 } },
    { '~',  { 0x00, 0x00, 0x09, 0x15, 0x12, 0x00, 0x00 } }
};

static const uint8_t *glyph_rows(char c) {
    if (c >= 'a' && c <= 'z') {
        c = (char)(c - 'a' + 'A');
    }

    for (size_t i = 0; i < sizeof(FONT5X7) / sizeof(FONT5X7[0]); i++) {
        if (FONT5X7[i].ch == c) {
            return FONT5X7[i].rows;
        }
    }
    return NULL;
}

static void draw_tiny_text(float x, float y, float scale, const char *text) {
    glColor3f(0.92f, 0.96f, 1.0f);
    glBegin(GL_QUADS);

    float cursor_x = x;
    for (const char *p = text; *p; p++) {
        const uint8_t *rows = glyph_rows(*p);
        if (rows) {
            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 5; col++) {
                    if (rows[row] & (uint8_t)(1u << (4 - col))) {
                        float px = cursor_x + (float)col * scale;
                        float py = y + (float)row * scale;
                        glVertex2f(px, py);
                        glVertex2f(px + scale, py);
                        glVertex2f(px + scale, py + scale);
                        glVertex2f(px, py + scale);
                    }
                }
            }
        }
        cursor_x += 6.0f * scale;
    }

    glEnd();
}

/* ------------------------------------------------------------------------- */
/* Sidebar UI primitives                                                     */
/* ------------------------------------------------------------------------- */

#define SIDEBAR_BTN_ID_CLOSE      1
#define SIDEBAR_BTN_ID_ADD        2
#define SIDEBAR_BTN_ID_CONTINUE   3
#define SIDEBAR_BTN_ID_CANCEL     4
#define SIDEBAR_BTN_ID_CONFIRM    5
#define SIDEBAR_BTN_ID_DENY       6
#define SIDEBAR_BTN_ID_BACK       7

static void draw_rect(float x, float y, float w, float h,
                      float r, float g, float b, float a) {
    (void)a;
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

static void draw_rect_outline(float x, float y, float w, float h,
                              float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

/* Returns true if (mx,my) is inside the rect. */
static bool rect_hit(float x, float y, float w, float h, float mx, float my) {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

/* Draw a labeled button. Returns its rect for hit testing. */
static void draw_button(float x, float y, float w, float h,
                        const char *label, bool hovered, bool active) {
    float r = hovered ? 0.35f : (active ? 0.25f : 0.18f);
    float g = hovered ? 0.55f : (active ? 0.45f : 0.28f);
    float b = hovered ? 0.85f : (active ? 0.75f : 0.48f);
    draw_rect(x, y, w, h, r, g, b, 1.0f);
    draw_rect_outline(x, y, w, h, 0.5f, 0.7f, 0.9f);

    float tw = (float)strlen(label) * 6.0f * 1.5f;
    float tx = x + (w - tw) * 0.5f;
    float ty = y + (h - 7.0f * 1.5f) * 0.5f;
    glColor3f(0.95f, 0.98f, 1.0f);
    draw_tiny_text(tx, ty, 1.5f, label);
}

/* A small text row with an optional trailing delete mark. */
static void draw_text(float x, float y, float scale, const char *text,
                      float r, float g, float b) {
    glColor3f(r, g, b);
    draw_tiny_text(x, y, scale, text);
}

/* ------------------------------------------------------------------------- */
/* Vsync / buffer helpers                                                    */
/* ------------------------------------------------------------------------- */

typedef void (*PFNGLXSWAPINTERVALEXTPROC)(Display *, GLXDrawable, int);
typedef int (*PFNGLXSWAPINTERVALMESAPROC)(unsigned int);
typedef int (*PFNGLXSWAPINTERVALSGIPROC)(int);

static bool load_glx_proc(const char *name, void *dest, size_t dest_size) {
    __GLXextFuncPtr fn = glXGetProcAddressARB((const GLubyte *)name);
    if (!fn) return false;
    memcpy(dest, &fn, dest_size < sizeof(fn) ? dest_size : sizeof(fn));
    return true;
}

static void set_swap_interval(Display *dpy, GLXDrawable drawable, int interval) {
    PFNGLXSWAPINTERVALEXTPROC ext = NULL;
    if (load_glx_proc("glXSwapIntervalEXT", &ext, sizeof(ext))) {
        ext(dpy, drawable, interval);
        return;
    }

    PFNGLXSWAPINTERVALMESAPROC mesa = NULL;
    if (load_glx_proc("glXSwapIntervalMESA", &mesa, sizeof(mesa))) {
        mesa((unsigned int)interval);
        return;
    }

    PFNGLXSWAPINTERVALSGIPROC sgi = NULL;
    if (load_glx_proc("glXSwapIntervalSGI", &sgi, sizeof(sgi))) {
        sgi(interval);
        return;
    }

    fprintf(stderr, "Warning: no GLX swap-interval extension; frame pacing may be uneven\n");
}

static bool ensure_point_buffer(float **data, int *capacity, int need) {
    if (need <= *capacity) return true;

    float *p = realloc(*data, (size_t)need * 5u * sizeof(float));
    if (!p) return false;

    *data = p;
    *capacity = need;
    return true;
}

/* ------------------------------------------------------------------------- */
/* Window / GL setup                                                         */
/* ------------------------------------------------------------------------- */

bool gfx_init(GfxCtx *gfx, int width, int height, const char *title) {
    memset(gfx, 0, sizeof(*gfx));
    gfx->width = width;
    gfx->height = height;
    gfx->dirty = true;
    gfx->sidebar_mode = SIDEBAR_CLOSED;
    gfx->sidebar_w = 0;
    gfx->sidebar_scroll = 0;
    gfx->hover_btn = -1;
    gfx->delete_marker_idx = -1;
    gfx->delete_channel_idx = -1;
    gfx->add_url_len = 0;
    gfx->add_url[0] = '\0';
    gfx->add_name_len = 0;
    gfx->add_name[0] = '\0';
    gfx->add_city_idx = -1;
    gfx->paste_len = 0;
    gfx->paste_buf[0] = '\0';
    gfx->paste_selection = None;
    gfx->paste_tried_string = false;
    gfx->blink_on = true;
    gfx->last_blink_time = 0.0;

    gfx->dpy = XOpenDisplay(NULL);
    if (!gfx->dpy) {
        fprintf(stderr, "Failed to open X11 display\n");
        return false;
    }

    int screen = DefaultScreen(gfx->dpy);
    GLint attribs[] = {
        GLX_RGBA,
        GLX_DEPTH_SIZE, 16,
        GLX_DOUBLEBUFFER,
        None
    };

    XVisualInfo *vi = glXChooseVisual(gfx->dpy, screen, attribs);
    if (!vi) {
        fprintf(stderr, "No suitable GLX visual found\n");
        XCloseDisplay(gfx->dpy);
        gfx->dpy = NULL;
        return false;
    }

    Colormap cmap = XCreateColormap(gfx->dpy, RootWindow(gfx->dpy, screen), vi->visual, AllocNone);
    XSetWindowAttributes swa;
    memset(&swa, 0, sizeof(swa));
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask |
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

    gfx->win = XCreateWindow(
        gfx->dpy, RootWindow(gfx->dpy, screen),
        0, 0, (unsigned int)width, (unsigned int)height, 0,
        vi->depth, InputOutput, vi->visual,
        CWColormap | CWEventMask, &swa
    );

    if (!gfx->win) {
        fprintf(stderr, "Failed to create X11 window\n");
        XFree(vi);
        XCloseDisplay(gfx->dpy);
        gfx->dpy = NULL;
        return false;
    }

    gfx->ibeam_cursor = XCreateFontCursor(gfx->dpy, XC_xterm);
    gfx->ibeam_active = false;
    gfx->mouse_x = gfx->mouse_y = -1;

    XStoreName(gfx->dpy, gfx->win, title);
    XMapWindow(gfx->dpy, gfx->win);

    gfx->wm_delete = XInternAtom(gfx->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(gfx->dpy, gfx->win, &gfx->wm_delete, 1);

    gfx->glx_ctx = glXCreateContext(gfx->dpy, vi, NULL, True);
    XFree(vi);

    if (!gfx->glx_ctx || !glXMakeCurrent(gfx->dpy, gfx->win, gfx->glx_ctx)) {
        fprintf(stderr, "Failed to create or bind GLX context\n");
        XDestroyWindow(gfx->dpy, gfx->win);
        XCloseDisplay(gfx->dpy);
        gfx->win = 0;
        gfx->dpy = NULL;
        return false;
    }

    set_swap_interval(gfx->dpy, gfx->win, 1);
    return true;
}

void gfx_shutdown(GfxCtx *gfx) {
    if (!gfx) return;

    free(gfx->land_data);
    free(gfx->marker_data);
    gfx->land_data = NULL;
    gfx->marker_data = NULL;

    if (!gfx->dpy) return;

    if (gfx->glx_ctx) {
        glXMakeCurrent(gfx->dpy, None, NULL);
        glXDestroyContext(gfx->dpy, gfx->glx_ctx);
        gfx->glx_ctx = NULL;
    }
    if (gfx->ibeam_cursor) {
        XFreeCursor(gfx->dpy, gfx->ibeam_cursor);
        gfx->ibeam_cursor = 0;
    }
    if (gfx->win) {
        XDestroyWindow(gfx->dpy, gfx->win);
        gfx->win = 0;
    }
    XCloseDisplay(gfx->dpy);
    gfx->dpy = NULL;
}

/* ------------------------------------------------------------------------- */
/* Events                                                                    */
/* ------------------------------------------------------------------------- */

static int find_hit_marker(
    const GfxCtx *gfx,
    float pitch_rad,
    float yaw_rad,
    const RadioMarker *markers,
    int marker_count,
    float mx,
    float my
) {
    float radius_px = 0.38f * (float)(gfx->width < gfx->height ? gfx->width : gfx->height);
    int best_idx = -1;
    float best_d2 = 1e30f;
    Rotation rot = rotation_make(pitch_rad, yaw_rad);

    for (int i = 0; i < marker_count; i++) {
        Vector3D p = rotateXYZ(markers[i].position, &rot);
        ScreenPoint sp = projectToScreen(p, gfx->width, gfx->height, radius_px);
        if (hit_test_point(sp, mx, my, 10.0f)) {
            float dx = sp.sx - mx;
            float dy = sp.sy - my;
            float d2 = dx * dx + dy * dy;
            if (d2 < best_d2) {
                best_d2 = d2;
                best_idx = i;
            }
        }
    }
    return best_idx;
}

static int sidebar_button_hit(GfxCtx *gfx, float mx, float my);

#define TEXT_SCALE 1.7f
#define URL_BOX_W 520.0f
#define URL_BOX_H 44.0f
#define URL_BOX_PAD 12.0f

static void url_box_rect(const GfxCtx *gfx, float *x, float *y, float *w, float *h) {
    *w = URL_BOX_W;
    *x = (float)gfx->width * 0.5f - URL_BOX_W * 0.5f;
    *y = 90.0f + 40.0f;
    *h = URL_BOX_H;
}

/* Max characters of the URL visible inside the text box. */
static int url_box_visible_chars(void) {
    return (int)((URL_BOX_W - 2.0f * URL_BOX_PAD) / (6.0f * TEXT_SCALE));
}

/* Show an I-beam while the pointer is over the URL text box. */
static void update_cursor(GfxCtx *gfx) {
    bool want = false;
    if (gfx->sidebar_mode == SIDEBAR_ADD_URL) {
        float bx, by, bw, bh;
        url_box_rect(gfx, &bx, &by, &bw, &bh);
        want = rect_hit(bx, by, bw, bh, (float)gfx->mouse_x, (float)gfx->mouse_y);
    }
    if (want == gfx->ibeam_active) return;
    gfx->ibeam_active = want;
    XDefineCursor(gfx->dpy, gfx->win, want ? gfx->ibeam_cursor : None);
    XFlush(gfx->dpy);
}

/* Ask the selection owner for text; result arrives as SelectionNotify. */
static void request_paste(GfxCtx *gfx, const char *selection_name, Atom target) {
    Atom sel = XInternAtom(gfx->dpy, selection_name, False);
    Atom prop = XInternAtom(gfx->dpy, "DOTRADIO_PASTE", False);
    gfx->paste_selection = sel;
    gfx->paste_tried_string = (target == XA_STRING);
    XConvertSelection(gfx->dpy, sel, target, prop, gfx->win, CurrentTime);
    XFlush(gfx->dpy);
}

static void paste_clipboard(GfxCtx *gfx) {
    request_paste(gfx, "CLIPBOARD", XInternAtom(gfx->dpy, "UTF8_STRING", False));
}

static void paste_primary(GfxCtx *gfx) {
    request_paste(gfx, "PRIMARY", XInternAtom(gfx->dpy, "UTF8_STRING", False));
}

static void handle_selection_notify(GfxCtx *gfx, const XSelectionEvent *se) {
    if (gfx->sidebar_mode != SIDEBAR_ADD_URL) return;

    if (se->property == None) {
        /* Owner refused UTF8_STRING; retry with plain STRING once. */
        if (!gfx->paste_tried_string) {
            request_paste(gfx, se->selection == XA_PRIMARY ? "PRIMARY" : "CLIPBOARD",
                          XA_STRING);
        }
        return;
    }

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    if (XGetWindowProperty(gfx->dpy, gfx->win, se->property, 0, 1024, True,
                           AnyPropertyType, &actual_type, &actual_format,
                           &nitems, &bytes_after, &prop) != Success || !prop) {
        return;
    }
    if (actual_format == 8) {
        for (unsigned long i = 0; i < nitems; i++) {
            unsigned char c = prop[i];
            if (c < 32 || c > 126) continue;                 /* drop newlines etc. */
            if (c == ' ' && gfx->add_url_len == 0) continue; /* trim leading space */
            if (gfx->add_url_len >= (int)sizeof(gfx->add_url) - 1) break;
            gfx->add_url[gfx->add_url_len++] = (char)c;
        }
        gfx->add_url[gfx->add_url_len] = '\0';
        while (gfx->add_url_len > 0 && gfx->add_url[gfx->add_url_len - 1] == ' ') {
            gfx->add_url[--gfx->add_url_len] = '\0';
        }
        gfx->dirty = true;
    }
    XFree(prop);
}

static void sidebar_draw(GfxCtx *gfx, const RadioMarker *markers, int marker_count);

static UiAction process_event(
    GfxCtx *gfx,
    XEvent *ev,
    float *pitch_rad,
    float *yaw_rad,
    const RadioMarker *markers,
    int marker_count,
    int *out_marker_idx
) {
    UiAction action = UI_ACTION_NONE;

    if (ev->type == MotionNotify) {
        gfx->mouse_x = ev->xmotion.x;
        gfx->mouse_y = ev->xmotion.y;
    } else if (ev->type == ButtonPress || ev->type == ButtonRelease) {
        gfx->mouse_x = ev->xbutton.x;
        gfx->mouse_y = ev->xbutton.y;
    }

    switch (ev->type) {
        case Expose:
            if (ev->xexpose.count == 0) {
                gfx->dirty = true;
            }
            break;

        case ConfigureNotify:
            if (gfx->width != ev->xconfigure.width || gfx->height != ev->xconfigure.height) {
                gfx->width = ev->xconfigure.width;
                gfx->height = ev->xconfigure.height;
                gfx->dirty = true;
            }
            break;

        case ClientMessage:
            if ((Atom)ev->xclient.data.l[0] == gfx->wm_delete) {
                return UI_ACTION_QUIT;
            }
            break;

        case KeyPress: {
            KeySym ks = XLookupKeysym(&ev->xkey, 0);

            /* Sidebar text input takes priority */
            if (gfx->sidebar_mode == SIDEBAR_ADD_URL) {
                char buf[8] = {0};
                XLookupString(&ev->xkey, buf, sizeof(buf), &ks, NULL);
                if (((ks == XK_v || ks == XK_V) && (ev->xkey.state & ControlMask)) ||
                    (ks == XK_Insert && (ev->xkey.state & ShiftMask))) {
                    paste_clipboard(gfx);
                } else if (ks == XK_Return || ks == XK_KP_Enter) {
                    if (gfx->add_url_len == 0) break;
                    strncpy(gfx->sidebar_action_url, gfx->add_url,
                            sizeof(gfx->sidebar_action_url) - 1);
                    gfx->sidebar_action_url[sizeof(gfx->sidebar_action_url) - 1] = '\0';
                    gfx->sidebar_action_city = -1;
                    gfx->sidebar_mode = SIDEBAR_ADD_CITY;
                    gfx->add_city_idx = -1;
                    gfx->sidebar_scroll = 0;
                    gfx->dirty = true;
                } else if (ks == XK_Escape) {
                    gfx->sidebar_mode = SIDEBAR_LIST;
                    gfx->dirty = true;
                } else if (ks == XK_BackSpace || ks == XK_Delete) {
                    if (gfx->add_url_len > 0) {
                        gfx->add_url[--gfx->add_url_len] = '\0';
                        gfx->dirty = true;
                    }
                } else if (!(ev->xkey.state & ControlMask) &&
                           buf[0] >= 32 && buf[0] < 127 &&
                           gfx->add_url_len < (int)sizeof(gfx->add_url) - 1) {
                    gfx->add_url[gfx->add_url_len++] = buf[0];
                    gfx->add_url[gfx->add_url_len] = '\0';
                    gfx->dirty = true;
                }
                break;
            }

            if (ks == XK_Escape || ks == XK_q) {
                if (gfx->sidebar_mode != SIDEBAR_CLOSED) {
                    gfx->sidebar_mode = SIDEBAR_CLOSED;
                    gfx->dirty = true;
                    break;
                }
                return UI_ACTION_QUIT;
            } else if (ks == XK_r || ks == XK_R) {
                action = UI_ACTION_RESCAN_STATIONS;
            } else if (ks == XK_s || ks == XK_S) {
                if (gfx->sidebar_mode == SIDEBAR_CLOSED) {
                    gfx->sidebar_mode = SIDEBAR_LIST;
                    gfx->sidebar_action = 3;
                } else {
                    gfx->sidebar_mode = SIDEBAR_CLOSED;
                    gfx->sidebar_action = 0;
                }
                gfx->dirty = true;
            }
            break;
        }

        case ButtonPress:
            if (ev->xbutton.button == Button1) {
                gfx->dragging = true;
                gfx->moved_while_down = false;
                gfx->press_x = gfx->last_x = ev->xbutton.x;
                gfx->press_y = gfx->last_y = ev->xbutton.y;
            }
            /* Scroll wheel for sidebar lists */
            if (ev->xbutton.button == 4 && gfx->sidebar_mode != SIDEBAR_CLOSED) {
                gfx->sidebar_scroll -= 40;
                if (gfx->sidebar_scroll < 0) gfx->sidebar_scroll = 0;
                gfx->dirty = true;
            }
            if (ev->xbutton.button == 5 && gfx->sidebar_mode != SIDEBAR_CLOSED) {
                gfx->sidebar_scroll += 40;
                gfx->dirty = true;
            }
            /* Middle click: paste PRIMARY; right click: paste CLIPBOARD */
            if (gfx->sidebar_mode == SIDEBAR_ADD_URL) {
                if (ev->xbutton.button == 2) paste_primary(gfx);
                else if (ev->xbutton.button == 3) paste_clipboard(gfx);
            }
            break;

        case MotionNotify:
            if (gfx->dragging) {
                int dx = ev->xmotion.x - gfx->last_x;
                int dy = ev->xmotion.y - gfx->last_y;
                gfx->last_x = ev->xmotion.x;
                gfx->last_y = ev->xmotion.y;

                int total_dx = ev->xmotion.x - gfx->press_x;
                int total_dy = ev->xmotion.y - gfx->press_y;
                if (total_dx * total_dx + total_dy * total_dy > 25) {
                    gfx->moved_while_down = true;
                }

                /* Sidebar drag scroll */
                if (gfx->sidebar_mode != SIDEBAR_CLOSED &&
                    ev->xmotion.x > (float)gfx->width - gfx->sidebar_w) {
                    gfx->sidebar_scroll += dy * 2;
                    if (gfx->sidebar_scroll < 0) gfx->sidebar_scroll = 0;
                    gfx->dirty = true;
                    break;
                }

                *yaw_rad += (float)dx * 0.008f;
                *pitch_rad += (float)dy * 0.008f;
                gfx->dirty = true;
            }
            break;

        case ButtonRelease:
            if (ev->xbutton.button == Button1 && gfx->dragging) {
                gfx->dragging = false;
                float mx = (float)ev->xbutton.x;
                float my = (float)ev->xbutton.y;

                /* Sidebar button hit first */
                if (gfx->sidebar_mode != SIDEBAR_CLOSED ||
                    rect_hit(10.0f, 10.0f, 120.0f, 30.0f, mx, my)) {
                    int btn = sidebar_button_hit(gfx, mx, my);
                    gfx->hover_btn = -1;
                    if (btn == 99) {
                        gfx->sidebar_mode = SIDEBAR_LIST;
                        gfx->sidebar_action = 3; /* toggle open */
                        gfx->dirty = true;
                    } else if (btn == SIDEBAR_BTN_ID_CLOSE) {
                        gfx->sidebar_mode = SIDEBAR_CLOSED;
                        gfx->sidebar_action = 0;
                        gfx->dirty = true;
                    } else if (btn == SIDEBAR_BTN_ID_ADD) {
                        gfx->sidebar_mode = SIDEBAR_ADD_URL;
                        gfx->add_url_len = 0;
                        gfx->add_url[0] = '\0';
                        gfx->add_name_len = 0;
                        gfx->add_name[0] = '\0';
                        gfx->add_city_idx = -1;
                        gfx->dirty = true;
                    } else if (btn == SIDEBAR_BTN_ID_CONTINUE) {
                        if (gfx->add_url_len == 0) break;
                        strncpy(gfx->sidebar_action_url, gfx->add_url,
                                sizeof(gfx->sidebar_action_url) - 1);
                        gfx->sidebar_action_url[sizeof(gfx->sidebar_action_url) - 1] = '\0';
                        gfx->sidebar_action_city = -1;
                        gfx->sidebar_mode = SIDEBAR_ADD_CITY;
                        gfx->add_city_idx = -1;
                        gfx->sidebar_scroll = 0;
                        gfx->dirty = true;
                    } else if (btn == SIDEBAR_BTN_ID_CANCEL) {
                        gfx->sidebar_mode = SIDEBAR_LIST;
                        gfx->dirty = true;
                    } else if (btn == SIDEBAR_BTN_ID_CONFIRM) {
                        if (gfx->sidebar_mode == SIDEBAR_ADD_CITY &&
                            gfx->add_city_idx >= 0) {
                            gfx->sidebar_action_city = gfx->add_city_idx;
                            gfx->sidebar_action = 2; /* add channel */
                            gfx->dirty = true;
                        } else if (gfx->sidebar_mode == SIDEBAR_CONFIRM_DELETE) {
                            gfx->sidebar_action = 1; /* delete */
                            gfx->dirty = true;
                        }
                    } else if (btn == SIDEBAR_BTN_ID_DENY) {
                        gfx->sidebar_mode = SIDEBAR_LIST;
                        gfx->delete_marker_idx = -1;
                        gfx->delete_channel_idx = -1;
                        gfx->dirty = true;
                    } else if (btn == SIDEBAR_BTN_ID_BACK) {
                        gfx->sidebar_mode = SIDEBAR_ADD_URL;
                        gfx->dirty = true;
                    } else if (btn >= 100) {
                        /* Per-row delete: decode i and j */
                        int code = btn - 100;
                        int mi = code / 16;
                        int cj = code % 16;
                        gfx->delete_marker_idx = mi;
                        gfx->delete_channel_idx = cj;
                        gfx->sidebar_mode = SIDEBAR_CONFIRM_DELETE;
                        gfx->dirty = true;
                    } else if (btn >= 200) {
                        gfx->add_city_idx = btn - 200;
                        gfx->dirty = true;
                    }
                    break;
                }

                if (!gfx->moved_while_down) {
                    int hit = find_hit_marker(
                        gfx, *pitch_rad, *yaw_rad,
                        markers, marker_count,
                        mx, my
                    );
                    if (hit >= 0 && out_marker_idx) {
                        *out_marker_idx = hit;
                        action = UI_ACTION_SELECT_OR_CYCLE_MARKER;
                    }
                }
            }
            break;

        case SelectionNotify:
            handle_selection_notify(gfx, &ev->xselection);
            break;
    }

    update_cursor(gfx);
    return action;
}

UiAction gfx_poll_events(
    GfxCtx *gfx,
    float *pitch_rad,
    float *yaw_rad,
    const RadioMarker *markers,
    int marker_count,
    int *out_marker_idx,
    bool block
) {
    UiAction action = UI_ACTION_NONE;
    if (out_marker_idx) *out_marker_idx = -1;

    if (block && XPending(gfx->dpy) == 0) {
        XEvent ev;
        XNextEvent(gfx->dpy, &ev);
        UiAction a = process_event(
            gfx, &ev, pitch_rad, yaw_rad, markers, marker_count, out_marker_idx
        );
        if (a == UI_ACTION_QUIT) return a;
        if (a != UI_ACTION_NONE) action = a;
    }

    while (XPending(gfx->dpy) > 0) {
        XEvent ev;
        XNextEvent(gfx->dpy, &ev);
        UiAction a = process_event(
            gfx, &ev, pitch_rad, yaw_rad, markers, marker_count, out_marker_idx
        );
        if (a == UI_ACTION_QUIT) return a;
        if (a != UI_ACTION_NONE) action = a;
    }

    return action;
}

/* ------------------------------------------------------------------------- */
/* Sidebar UI                                                                */
/* ------------------------------------------------------------------------- */

#define SIDEBAR_TITLE "Radio Stations"
#define ROW_H 44.0f

static int sidebar_button_hit(GfxCtx *gfx, float mx, float my) {
    if (gfx->sidebar_mode == SIDEBAR_CLOSED) {
        /* Small toggle button top-left */
        if (rect_hit(10.0f, 10.0f, 120.0f, 30.0f, mx, my)) return 99;
        return -1;
    }

    float cx = (float)gfx->width;
    float cy = (float)gfx->height;
    float mx_ = mx, my_ = my;

    /* Close button (top-right corner) */
    if (rect_hit(cx - 60.0f, 10.0f, 50.0f, 30.0f, mx_, my_)) return SIDEBAR_BTN_ID_CLOSE;

    if (gfx->sidebar_mode == SIDEBAR_LIST) {
        /* "+ Add Channel" button bottom-center */
        float bw = 220.0f, bh = 44.0f;
        float bx = cx * 0.5f - bw * 0.5f;
        float by = cy - 80.0f;
        if (rect_hit(bx, by, bw, bh, mx_, my_)) return SIDEBAR_BTN_ID_ADD;

        /* Per-row delete buttons (right side of each row) */
        int marker_count = 0;
        const RadioMarker *markers = world_get_markers(&marker_count);
        float ry = 120.0f;
        for (int i = 0; i < marker_count; i++) {
            const RadioMarker *m = &markers[i];
            for (int j = 0; j < m->num_channels; j++) {
                float dw = 50.0f, dh = ROW_H * 0.7f;
                float dx = cx - dw - 60.0f;
                float dy = ry + (ROW_H - dh) * 0.5f;
                if (rect_hit(dx, dy, dw, dh, mx_, my_)) {
                    return 100 + i * 16 + j;
                }
                ry += ROW_H;
            }
        }
        return -1;
    }

    if (gfx->sidebar_mode == SIDEBAR_ADD_URL) {
        float bw = 160.0f, bh = 44.0f;
        float bx = cx * 0.5f - bw * 0.5f - bw - 10.0f;
        float by = cy - 140.0f;
        if (rect_hit(bx, by, bw, bh, mx_, my_)) return SIDEBAR_BTN_ID_CONTINUE;
        bx = cx * 0.5f + 10.0f;
        if (rect_hit(bx, by, bw, bh, mx_, my_)) return SIDEBAR_BTN_ID_CANCEL;

        return -1;
    }

    if (gfx->sidebar_mode == SIDEBAR_ADD_CITY) {
        /* City row clicks (full width) */
        int city_count = world_city_count();
        float ry = 140.0f;
        for (int i = 0; i < city_count; i++) {
            const City *c = world_city_list(i);
            if (!c) continue;
            float dw = cx - 120.0f, dh = ROW_H;
            float dx = 60.0f;
            float dy = ry;
            if (rect_hit(dx, dy, dw, dh, mx_, my_)) {
                return 200 + i; /* city row id */
            }
            ry += ROW_H;
        }

        float bw = 160.0f, bh = 44.0f;
        float bx = cx * 0.5f - bw * 0.5f - bw - 10.0f;
        float by = cy - 100.0f;
        if (rect_hit(bx, by, bw, bh, mx_, my_)) return SIDEBAR_BTN_ID_CONFIRM;
        bx = cx * 0.5f + 10.0f;
        if (rect_hit(bx, by, bw, bh, mx_, my_)) return SIDEBAR_BTN_ID_BACK;
        return -1;
    }

    if (gfx->sidebar_mode == SIDEBAR_CONFIRM_DELETE) {
        float bw = 120.0f, bh = 44.0f;
        float bx = cx * 0.5f - bw * 0.5f - bw - 10.0f;
        float by = cy * 0.5f + 80.0f;
        if (rect_hit(bx, by, bw, bh, mx_, my_)) return SIDEBAR_BTN_ID_CONFIRM;
        bx = cx * 0.5f + 10.0f;
        if (rect_hit(bx, by, bw, bh, mx_, my_)) return SIDEBAR_BTN_ID_DENY;
        return -1;
    }

    return -1;
}

static void sidebar_draw(GfxCtx *gfx, const RadioMarker *markers, int marker_count) {
    float cx = (float)gfx->width;
    float cy = (float)gfx->height;

    if (gfx->sidebar_mode == SIDEBAR_CLOSED) {
        /* Small toggle button top-left */
        bool hovered = gfx->hover_btn == 99;
        draw_button(10.0f, 10.0f, 120.0f, 30.0f, "Radio Stations", hovered, false);
        return;
    }

    /* Full-screen overlay background */
    draw_rect(0.0f, 0.0f, cx, cy, 0.05f, 0.07f, 0.10f, 1.0f);
    draw_rect_outline(0.0f, 0.0f, cx, 2.0f, 0.4f, 0.6f, 0.9f);
    draw_rect_outline(0.0f, cy - 2.0f, cx, 2.0f, 0.4f, 0.6f, 0.9f);

    /* Title bar */
    draw_rect(0.0f, 0.0f, cx, 60.0f, 0.12f, 0.18f, 0.28f, 1.0f);
    glColor3f(0.95f, 0.98f, 1.0f);
    draw_tiny_text(20.0f, 18.0f, TEXT_SCALE, SIDEBAR_TITLE);
    /* Close button top-right */
    draw_button(cx - 70.0f, 14.0f, 50.0f, 32.0f, "Close", true, false);

    float y = 90.0f;

    if (gfx->sidebar_mode == SIDEBAR_LIST) {
        float ry = y;
        for (int i = 0; i < marker_count; i++) {
            const RadioMarker *m = &markers[i];
            for (int j = 0; j < m->num_channels; j++) {
                const RadioChannel *ch = &m->channels[j];
                if (ry < cy - 160.0f) {
                    float r = 0.92f, g = 0.96f, b = 1.0f;
                    if (gfx->hover_btn == 100 + i * 16 + j) {
                        draw_rect(40.0f, ry - 6.0f, cx - 200.0f, ROW_H, 0.2f, 0.35f, 0.55f, 1.0f);
                        r = 1.0f; g = 0.85f; b = 0.5f;
                    }
                    char line[160];
                    snprintf(line, sizeof(line), "%s — %s (%s)", m->city, ch->name, m->city);
                    draw_text(60.0f, ry + 6.0f, TEXT_SCALE, line, r, g, b);
                    /* Delete mark */
                    draw_text(cx - 90.0f, ry + 6.0f, TEXT_SCALE, "x", 1.0f, 0.35f, 0.35f);
                }
                ry += ROW_H;
            }
        }

        float bw = 240.0f, bh = 48.0f;
        float bx = cx * 0.5f - bw * 0.5f;
        float by = cy - 100.0f;
        draw_button(bx, by, bw, bh, "+ Add Channel",
                    gfx->hover_btn == SIDEBAR_BTN_ID_ADD, false);
    }
    else if (gfx->sidebar_mode == SIDEBAR_ADD_URL) {
        draw_text(60.0f, y, TEXT_SCALE, "Enter stream URL:", 0.9f, 0.9f, 1.0f);

        /* Text box (centered) */
        float tb_x, tb_y, tb_w, tb_h;
        url_box_rect(gfx, &tb_x, &tb_y, &tb_w, &tb_h);
        draw_rect(tb_x, tb_y, tb_w, tb_h, 0.16f, 0.22f, 0.32f, 1.0f);
        draw_rect_outline(tb_x, tb_y, tb_w, tb_h, 0.4f, 0.6f, 0.9f);

        /* Show the tail of the URL so the caret stays inside the box. */
        int vis = url_box_visible_chars();
        int off = gfx->add_url_len > vis ? gfx->add_url_len - vis : 0;
        float text_y = tb_y + (tb_h - 7.0f * TEXT_SCALE) * 0.5f;
        glColor3f(0.95f, 0.98f, 1.0f);
        draw_tiny_text(tb_x + URL_BOX_PAD, text_y, TEXT_SCALE, gfx->add_url + off);

        /* Text caret (vertical line after the last character) */
        float caret_x = tb_x + URL_BOX_PAD +
                        (float)(gfx->add_url_len - off) * 6.0f * TEXT_SCALE;
        draw_rect(caret_x, text_y - 4.0f, 2.0f, 7.0f * TEXT_SCALE + 8.0f,
                  0.95f, 0.98f, 1.0f, 1.0f);

        draw_text(tb_x, tb_y + tb_h + 12.0f, 1.2f,
                  "Ctrl+V or right-click to paste",
                  0.6f, 0.7f, 0.85f);

        /* Continue / Cancel */
        float bw = 160.0f, bh = 48.0f;
        float bx = cx * 0.5f - bw * 0.5f - bw - 20.0f;
        float by = cy - 140.0f;
        draw_button(bx, by, bw, bh, "Continue",
                    gfx->hover_btn == SIDEBAR_BTN_ID_CONTINUE, false);
        bx = cx * 0.5f + 20.0f;
        draw_button(bx, by, bw, bh, "Cancel",
                    gfx->hover_btn == SIDEBAR_BTN_ID_CANCEL, false);
    }
    else if (gfx->sidebar_mode == SIDEBAR_ADD_CITY) {
        draw_text(60.0f, y, TEXT_SCALE, "Choose city:", 0.9f, 0.9f, 1.0f);
        int city_count = world_city_count();
        float ry = y + 50.0f;
        int shown = 0;
        for (int i = 0; i < city_count; i++) {
            const City *c = world_city_list(i);
            if (!c) continue;
            if (ry < cy - 160.0f) {
                bool selected = (i == gfx->add_city_idx);
                if (selected) {
                    draw_rect(40.0f, ry - 6.0f, cx - 200.0f, ROW_H, 0.2f, 0.35f, 0.55f, 1.0f);
                }
                float r = selected ? 1.0f : 0.85f;
                float g = selected ? 0.9f : 0.85f;
                float b = selected ? 0.4f : 0.85f;
                char line[120];
                snprintf(line, sizeof(line), "%s, %s", c->name, c->country);
                draw_text(60.0f, ry + 6.0f, TEXT_SCALE, line, r, g, b);
                shown++;
            }
            ry += ROW_H;
        }
        if (shown == 0) {
            draw_text(60.0f, y + 60.0f, TEXT_SCALE, "No cities", 0.7f, 0.7f, 0.7f);
        }

        float bw = 160.0f, bh = 48.0f;
        float bx = cx * 0.5f - bw * 0.5f - bw - 20.0f;
        float by = cy - 120.0f;
        int can_confirm = gfx->add_city_idx >= 0;
        draw_button(bx, by, bw, bh, "Confirm",
                    gfx->hover_btn == SIDEBAR_BTN_ID_CONFIRM && can_confirm,
                    can_confirm);
        bx = cx * 0.5f + 20.0f;
        draw_button(bx, by, bw, bh, "Back",
                    gfx->hover_btn == SIDEBAR_BTN_ID_BACK, false);
    }
    else if (gfx->sidebar_mode == SIDEBAR_CONFIRM_DELETE) {
        draw_text(cx * 0.5f - 160.0f, cy * 0.5f - 40.0f, TEXT_SCALE,
                  "Remove this channel?", 1.0f, 0.9f, 0.8f);
        float bw = 140.0f, bh = 48.0f;
        float bx = cx * 0.5f - bw * 0.5f - bw - 20.0f;
        float by = cy * 0.5f + 40.0f;
        draw_button(bx, by, bw, bh, "Yes",
                    gfx->hover_btn == SIDEBAR_BTN_ID_CONFIRM, false);
        bx = cx * 0.5f + 20.0f;
        draw_button(bx, by, bw, bh, "No",
                    gfx->hover_btn == SIDEBAR_BTN_ID_DENY, false);
    }
}

/* ------------------------------------------------------------------------- */
/* Rendering                                                                 */
/* ------------------------------------------------------------------------- */

void gfx_render(
    GfxCtx *gfx,
    float pitch_rad,
    float yaw_rad,
    const Vector3D *land_pts,
    int land_count,
    const RadioMarker *markers,
    int marker_count,
    int selected_marker
) {
    glViewport(0, 0, gfx->width, gfx->height);
    glClearColor(0.03f, 0.05f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)gfx->width, (double)gfx->height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radius_px = 0.38f * (float)(gfx->width < gfx->height ? gfx->width : gfx->height);

    bool camera_changed =
        !gfx->land_cache_valid ||
        pitch_rad != gfx->land_cache_pitch ||
        yaw_rad != gfx->land_cache_yaw ||
        gfx->width != gfx->land_cache_width ||
        gfx->height != gfx->land_cache_height ||
        land_count != gfx->land_point_count;

    if (camera_changed) {
        if (!ensure_point_buffer(&gfx->land_data, &gfx->land_capacity, land_count)) {
            fprintf(stderr, "Out of memory while caching land points\n");
            gfx->land_visible = 0;
            gfx->land_cache_valid = false;
        } else {
            Rotation rot = rotation_make(pitch_rad, yaw_rad);
            int visible = 0;

            for (int i = 0; i < land_count; i++) {
                Vector3D p = rotateXYZ(land_pts[i], &rot);
                ScreenPoint sp = projectToScreen(p, gfx->width, gfx->height, radius_px);
                if (!sp.visible) continue;

                float shade = 0.35f + 0.55f * sp.depth;
                float *v = &gfx->land_data[visible * 5];
                v[0] = sp.sx;
                v[1] = sp.sy;
                v[2] = 0.15f * shade;
                v[3] = 0.75f * shade;
                v[4] = 0.55f * shade;
                visible++;
            }

            gfx->land_visible = visible;
            gfx->land_point_count = land_count;
            gfx->land_cache_pitch = pitch_rad;
            gfx->land_cache_yaw = yaw_rad;
            gfx->land_cache_width = gfx->width;
            gfx->land_cache_height = gfx->height;
            gfx->land_cache_valid = true;
        }
    }

    if (gfx->land_visible > 0) {
        glPointSize(2.0f);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(2, GL_FLOAT, (GLsizei)(5 * sizeof(float)), gfx->land_data);
        glColorPointer(3, GL_FLOAT, (GLsizei)(5 * sizeof(float)), gfx->land_data + 2);
        glDrawArrays(GL_POINTS, 0, gfx->land_visible);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    /* Radio station markers */
    if (marker_count > 0 &&
        ensure_point_buffer(&gfx->marker_data, &gfx->marker_capacity, marker_count)) {
        Rotation rot = rotation_make(pitch_rad, yaw_rad);
        int visible = 0;

        for (int i = 0; i < marker_count; i++) {
            Vector3D p = rotateXYZ(markers[i].position, &rot);
            ScreenPoint sp = projectToScreen(p, gfx->width, gfx->height, radius_px);
            if (!sp.visible) continue;

            float *v = &gfx->marker_data[visible * 5];
            v[0] = sp.sx;
            v[1] = sp.sy;
            if (i == selected_marker) {
                v[2] = 1.0f;
                v[3] = 0.85f;
                v[4] = 0.15f;
            } else {
                v[2] = 1.0f;
                v[3] = 0.35f;
                v[4] = 0.25f;
            }
            visible++;
        }

        if (visible > 0) {
            glPointSize(7.0f);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            glVertexPointer(2, GL_FLOAT, (GLsizei)(5 * sizeof(float)), gfx->marker_data);
            glColorPointer(3, GL_FLOAT, (GLsizei)(5 * sizeof(float)), gfx->marker_data + 2);
            glDrawArrays(GL_POINTS, 0, visible);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
        }
    }

    /* Corner channel HUD */
    if (selected_marker >= 0 && selected_marker < marker_count) {
        const RadioMarker *m = &markers[selected_marker];
        if (m->num_channels > 0 && m->channels &&
            m->active_channel_index >= 0 && m->active_channel_index < m->num_channels) {
            const RadioChannel *ch = &m->channels[m->active_channel_index];
            char line[160];
            snprintf(line, sizeof(line), "%s [%d/%d] %s",
                     m->city, m->active_channel_index + 1, m->num_channels, ch->name);
            draw_tiny_text(12.0f, 14.0f, 2.0f, line);
        }
    }

    /* Sidebar overlay */
    if (gfx->sidebar_mode != SIDEBAR_CLOSED) {
        sidebar_draw(gfx, markers, marker_count);
    }

    glXSwapBuffers(gfx->dpy, gfx->win);
    gfx->dirty = false;
}
