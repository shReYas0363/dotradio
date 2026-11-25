#include "audioproc.h"
#include "gfx_x11.h"
#include "world_data.h"
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

static bool prompt_remove_channel(const char *city, const char *station) {
    fprintf(stderr,
            "\n\u2717 Station '%s' in '%s' failed to play. Remove it? "
            "(\u2713/y = yes, \u2717/n = no): ",
            station, city);
    fflush(stderr);

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    struct timeval tv = { 10, 0 }; /* 10 second timeout */
    int r = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

    if (r <= 0) {
        fprintf(stderr, "\nTimeout. Keeping channel.\n");
        return false;
    }

    int c = fgetc(stdin);
    while (c != '\n' && c != EOF) c = fgetc(stdin);

    return (c == 'y' || c == 'Y' || c == '\n'); /* Enter = yes */
}

/*
 * Returns true if playback started, false if it failed.
 * On failure, prompts the user to remove the channel.
 */
static bool handle_marker_action(RadioMarker *markers, int marker_count, int hit_idx,
                                 int *selected_marker, AudioProc *audio,
                                 const char *stations_path) {
    if (hit_idx < 0 || hit_idx >= marker_count) return false;

    RadioMarker *m = &markers[hit_idx];
    if (m->num_channels <= 0) return false;

    if (*selected_marker == hit_idx) {
        m->active_channel_index = (m->active_channel_index + 1) % m->num_channels;
    } else {
        *selected_marker = hit_idx;
        if (m->active_channel_index < 0 || m->active_channel_index >= m->num_channels) {
            m->active_channel_index = 0;
        }
    }

    const RadioChannel *ch = &m->channels[m->active_channel_index];
    printf("Tuning %s [%d/%d]: %s (%s)\n",
           m->city, m->active_channel_index + 1, m->num_channels, ch->name, ch->url);
    fflush(stdout);

    if (!audio_play_url(audio, ch->url)) {
        if (prompt_remove_channel(m->city, ch->name)) {
            int removed = world_remove_channel(hit_idx, m->active_channel_index);
            if (removed < 0) {
                /* Marker removed entirely; deselect if it was selected. */
                if (*selected_marker == hit_idx) *selected_marker = -1;
            } else if (removed != hit_idx) {
                /* Marker index shifted; adjust selection. */
                if (*selected_marker == hit_idx) *selected_marker = removed;
            }
            world_save_stations(stations_path);
        }
        return false;
    }

    return true;
}

int main(void) {
    world_init();

    int land_count = 0;
    const Vector3D *land_pts = world_get_land_points(&land_count);

    int marker_count = 0;
    RadioMarker *markers = world_get_markers(&marker_count);

    GfxCtx gfx;
    if (!gfx_init(&gfx, 480, 480, "Dot Globe Radio")) {
        world_shutdown();
        return 1;
    }

    AudioProc audio;
    audio_init(&audio);

    float pitch_rad = 0.25f;
    float yaw_rad = -0.5f;
    int selected_marker = -1;
    bool running = true;
    const char *stations_path = "stations.csv";

    while (running) {
        int hit_marker = -1;
        UiAction action = gfx_poll_events(
            &gfx, &pitch_rad, &yaw_rad,
            markers, marker_count, &hit_marker,
            !gfx.dirty
        );

        if (action == UI_ACTION_QUIT) {
            running = false;
        } else if (action == UI_ACTION_SELECT_OR_CYCLE_MARKER) {
            handle_marker_action(markers, marker_count, hit_marker,
                                 &selected_marker, &audio, stations_path);
            /* Refresh pointers in case world_remove_channel realloc'd. */
            markers = world_get_markers(&marker_count);
            gfx.dirty = true;
        } else if (action == UI_ACTION_RESCAN_STATIONS) {
            if (world_reload_stations() == 0) {
                markers = world_get_markers(&marker_count);
                land_pts = world_get_land_points(&land_count);
                printf("Stations reloaded: %d markers\n", marker_count);
            } else {
                fprintf(stderr, "Stations reload failed; keeping current list\n");
            }
            fflush(stdout);
            gfx.dirty = true;
        }

        /* Sidebar actions */
        if (gfx.sidebar_action == 1) {
            /* Delete channel */
            int mi = gfx.sidebar_action_marker;
            int ci = gfx.sidebar_action_channel;
            int removed = world_remove_channel(mi, ci);
            if (removed >= 0) {
                /* Marker still exists; selection index may have shifted */
                if (selected_marker == mi) selected_marker = removed;
            } else {
                /* Marker removed entirely */
                if (selected_marker == mi) selected_marker = -1;
                else if (selected_marker > mi) selected_marker--;
            }
            world_save_stations("stations.csv");
            markers = world_get_markers(&marker_count);
            gfx.sidebar_mode = SIDEBAR_LIST;
            gfx.sidebar_action = 0;
            gfx.dirty = true;
        } else if (gfx.sidebar_action == 2) {
            /* Add channel */
            const City *c = NULL;
            if (gfx.sidebar_action_city >= 0) {
                c = world_city_list(gfx.sidebar_action_city);
            }
            if (c && gfx.sidebar_action_url[0] != '\0') {
                world_append_station(c->name, c->lat, c->lon,
                                     "Unknown Station", gfx.sidebar_action_url);
                markers = world_get_markers(&marker_count);
                land_pts = world_get_land_points(&land_count);
            }
            gfx.sidebar_mode = SIDEBAR_LIST;
            gfx.sidebar_action = 0;
            gfx.add_url_len = 0;
            gfx.add_url[0] = '\0';
            gfx.add_city_idx = -1;
            gfx.dirty = true;
        }

        audio_poll(&audio);

        if (gfx.dirty) {
            gfx_render(&gfx, pitch_rad, yaw_rad,
                       land_pts, land_count,
                       markers, marker_count, selected_marker);
        }
    }

    audio_shutdown(&audio);
    gfx_shutdown(&gfx);
    world_shutdown();
    return 0;
}
