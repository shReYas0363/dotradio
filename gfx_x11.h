#ifndef GFX_X11_H
#define GFX_X11_H

#include "world_data.h"
#include <stdbool.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

typedef enum {
    UI_ACTION_NONE = 0,
    UI_ACTION_SELECT_OR_CYCLE_MARKER,
    UI_ACTION_RESCAN_STATIONS,
    UI_ACTION_QUIT
} UiAction;

typedef enum {
    SIDEBAR_CLOSED = 0,
    SIDEBAR_LIST,
    SIDEBAR_ADD_URL,
    SIDEBAR_ADD_CITY,
    SIDEBAR_CONFIRM_DELETE
} SidebarMode;

typedef struct {
    Display *dpy;
    Window win;
    GLXContext glx_ctx;
    Atom wm_delete;
    int width;
    int height;
    bool dragging;
    bool moved_while_down;
    int press_x;
    int press_y;
    int last_x;
    int last_y;

    bool dirty;

    /* Cached, interleaved x,y,r,g,b point data. */
    float *land_data;
    int land_capacity;
    int land_visible;
    int land_point_count;
    float land_cache_pitch;
    float land_cache_yaw;
    int land_cache_width;
    int land_cache_height;
    bool land_cache_valid;

    float *marker_data;
    int marker_capacity;

    /* ------------------------------------------------------------------ */
    /* Sidebar UI                                                         */
    /* ------------------------------------------------------------------ */
    SidebarMode sidebar_mode;
    int sidebar_w;                 /* panel width in pixels */
    int sidebar_scroll;            /* scroll offset for list/city picker */
    int hover_btn;                 /* button under mouse (-1 = none) */

    int delete_marker_idx;         /* marker pending deletion */
    int delete_channel_idx;        /* channel pending deletion */

    char add_url[256];
    int add_url_len;
    char add_name[128];
    int add_name_len;
    int add_city_idx;              /* selected city index in add-city mode */
    char paste_buf[512];           /* clipboard paste buffer */
    int paste_len;                 /* paste buffer length */
    Atom paste_selection;          /* selection last requested for paste */
    bool paste_tried_string;       /* STRING fallback already requested */
    Cursor ibeam_cursor;           /* I-beam pointer for text input */
    bool ibeam_active;
    int mouse_x, mouse_y;          /* last known pointer position */
    bool blink_on;                 /* text cursor blink state */
    double last_blink_time;        /* last blink toggle time */

    /* Pending actions for main.c to handle */
    int sidebar_action;            /* 0 none, 1 delete, 2 add, 3 toggle */
    int sidebar_action_marker;     /* marker index for delete */
    int sidebar_action_channel;    /* channel index for delete */
    char sidebar_action_url[256];  /* url for add */
    int sidebar_action_city;       /* city index for add */
} GfxCtx;

bool gfx_init(GfxCtx *gfx, int width, int height, const char *title);
void gfx_shutdown(GfxCtx *gfx);

/* Poll pending X11 events, update pitch/yaw on drag, and return any discrete action.
 * If UI_ACTION_SELECT_OR_CYCLE_MARKER is returned, *out_marker_idx holds the hit marker.
 * When block is true and no events are pending, this waits for the next event. */
UiAction gfx_poll_events(
    GfxCtx *gfx,
    float *pitch_rad,
    float *yaw_rad,
    const RadioMarker *markers,
    int marker_count,
    int *out_marker_idx,
    bool block
);

/* Render dot-matrix globe, markers, and corner channel HUD */
void gfx_render(
    GfxCtx *gfx,
    float pitch_rad,
    float yaw_rad,
    const Vector3D *land_pts,
    int land_count,
    const RadioMarker *markers,
    int marker_count,
    int selected_marker
);

#endif
