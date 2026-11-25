#pragma once
#include "globemath.h"
#include "cities.h"

typedef struct {
    const char* name;
    const char* url;
} RadioChannel;

typedef struct{
    const char* city;
    float lat;
    float lon;
    Vector3D position;
    int num_channels;  
    int active_channel_index;
    RadioChannel* channels;
} RadioMarker;

const Vector3D *world_get_land_points(int *out_land_count);
RadioMarker *world_get_markers(int *out_marker_count);
void world_init(void);
void world_shutdown(void);

/*
 * Remove one channel from a marker. Returns 0 on success, -1 on bad index.
 * When a marker's last channel is removed, the marker itself is removed too.
 * Returns the index of the removed marker (or -1 if the marker is gone).
 */
int world_remove_channel(int marker_idx, int channel_idx);

int world_reload_stations(void);

int world_save_stations(const char *path);

/*
 * Append a new station to the CSV file and reload the station list.
 * Returns 0 on success, -1 on error.
 */
int world_append_station(const char *city, float lat, float lon,
                         const char *station, const char *url);

/*
 * Read-only access to the bundled city database (for the add-channel city picker).
 */
int world_city_count(void);
const City *world_city_list(int index);
