#define _POSIX_C_SOURCE 200809L

#include "world_data.h"
#include "cities.h"
#include "landmask.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_LAND_PTS 8192

/*
 * Station data is loaded from a CSV file at startup:
 *
 *   city,latitude,longitude,station name,stream URL
 *
 * One row per station; rows for the same city are grouped automatically.
 * See stations.csv for the format and the default list.
 */
typedef struct {
    const char *city;
    float lat;
    float lon;
    const char *station;
    const char *url;
} DefaultStation;

static const DefaultStation DEFAULT_STATIONS[] = {
    { "London",    51.5074f,  -0.1278f, "BBC World Service", "https://stream.live.vc.bbcmedia.co.uk/bbc_world_service" },
    { "London",    51.5074f,  -0.1278f, "NTS Radio 1",      "https://stream-relay-geo.ntslive.net/stream" },
    { "London",    51.5074f,  -0.1278f, "Resonance FM",     "https://stream.resonance.fm/resonance" },
    { "Tokyo",     35.6762f, 139.6503f, "J-Pop Sakura",     "https://cast1.torontocast.com:2170/stream" },
    { "Tokyo",     35.6762f, 139.6503f, "Shonan Beach FM",  "http://shonanbeachfm.out.airtime.pro:8000/shonanbeachfm_a" },
    { "New York",  40.7128f, -74.0060f, "WNYC 93.9 FM",     "https://fm939.wnyc.org/wnycfm" },
    { "New York",  40.7128f, -74.0060f, "WKCR 89.9 FM",     "https://wkcr.streamguys1.com/live" },
    { "Reykjavik", 64.1466f, -21.9426f, "RUV Ras 2",        "http://netradio.ruv.is/ras2.mp3" },
};

static RadioMarker *MARKERS = NULL;
static int MARKER_COUNT = 0;
static int MARKER_CAPACITY = 0;

static Vector3D LAND_PTS[MAX_LAND_PTS];
static int LAND_COUNT = 0;

static void trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);

    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

/*
 * Split one CSV line in place. Supports double-quoted fields (with "" as an
 * escaped quote) and trims surrounding whitespace. Returns the number of
 * fields, or -1 for a malformed line.
 */
static int split_csv(char *line, char *fields[], int max_fields) {
    int count = 0;
    char *p = line;

    while (count < max_fields) {
        while (*p == ' ' || *p == '\t') p++;

        char *field = p;
        char *w = p;
        bool quoted = false;

        if (*p == '"') {
            quoted = true;
            p++;
        }

        while (*p) {
            if (quoted) {
                if (*p == '"') {
                    if (p[1] == '"') {
                        *w++ = '"';
                        p += 2;
                        continue;
                    }
                    quoted = false;
                    p++;
                    continue;
                }
                *w++ = *p++;
            } else if (*p == ',') {
                break;
            } else {
                *w++ = *p++;
            }
        }

        if (quoted) return -1;

        char delimiter = *p;
        *w = '\0';
        trim(field);
        fields[count++] = field;

        if (delimiter == ',') {
            p++;
            if (count == max_fields) {
                if (*p != '\0') return -2;
                break;
            }
            if (*p == '\0') {
                fields[count++] = p;
                break;
            }
        } else {
            break;
        }
    }

    return count;
}

static int find_marker(const char *city) {
    for (int i = 0; i < MARKER_COUNT; i++) {
        if (strcmp(MARKERS[i].city, city) == 0) return i;
    }
    return -1;
}

static bool ensure_marker_capacity(int needed) {
    if (needed <= MARKER_CAPACITY) return true;

    int capacity = MARKER_CAPACITY > 0 ? MARKER_CAPACITY : 8;
    while (capacity < needed) capacity *= 2;

    RadioMarker *grown = realloc(MARKERS, (size_t)capacity * sizeof(*grown));
    if (!grown) return false;

    MARKERS = grown;
    MARKER_CAPACITY = capacity;
    return true;
}

static bool marker_add_station(const char *city, float lat, float lon,
                               const char *station, const char *url) {
    int index = find_marker(city);

    if (index >= 0) {
        const RadioMarker *existing = &MARKERS[index];
        if (fabsf(existing->lat - lat) > 0.0001f ||
            fabsf(existing->lon - lon) > 0.0001f) {
            fprintf(stderr,
                    "dotradio: stations: '%s' appears with conflicting coordinates; "
                    "keeping %.4f,%.4f\n",
                    city, existing->lat, existing->lon);
            return true;
        }
    } else {
        if (!ensure_marker_capacity(MARKER_COUNT + 1)) return false;

        RadioMarker *marker = &MARKERS[MARKER_COUNT];
        memset(marker, 0, sizeof(*marker));

        marker->city = strdup(city);
        if (!marker->city) return false;

        marker->lat = lat;
        marker->lon = lon;
        marker->position = geoToSphere(lat, lon);
        marker->num_channels = 0;
        marker->active_channel_index = 0;
        marker->channels = NULL;

        index = MARKER_COUNT++;
    }

    RadioMarker *marker = &MARKERS[index];
    RadioChannel *grown = realloc(
        marker->channels,
        (size_t)(marker->num_channels + 1) * sizeof(*grown)
    );
    if (!grown) return false;

    marker->channels = grown;

    char *name_copy = strdup(station);
    char *url_copy = strdup(url);
    if (!name_copy || !url_copy) {
        free(name_copy);
        free(url_copy);
        return false;
    }

    marker->channels[marker->num_channels].name = name_copy;
    marker->channels[marker->num_channels].url = url_copy;
    marker->num_channels++;
    return true;
}

static bool load_stations_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) return false;

    char *line = NULL;
    size_t capacity = 0;
    int line_number = 0;
    int valid_rows = 0;

    while (getline(&line, &capacity, file) != -1) {
        line_number++;

        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (line_number == 1 && len >= 3 &&
            (unsigned char)line[0] == 0xEF &&
            (unsigned char)line[1] == 0xBB &&
            (unsigned char)line[2] == 0xBF) {
            memmove(line, line + 3, len - 2);
        }

        char *start = line;
        while (*start && isspace((unsigned char)*start)) start++;
        if (*start == '\0' || *start == '#') continue;

        char *fields[5];
        int count = split_csv(line, fields, 5);
        if (count == -2) {
            fprintf(stderr, "dotradio: %s:%d: too many fields "
                    "(expected 5)\n", path, line_number);
            continue;
        }
        if (count < 0) {
            fprintf(stderr, "dotradio: %s:%d: malformed quoted field\n",
                    path, line_number);
            continue;
        }
        if (count != 5) {
            fprintf(stderr,
                    "dotradio: %s:%d: expected 5 fields "
                    "(city,lat,lon,station,url)\n",
                    path, line_number);
            continue;
        }

        /* Optional header row. */
        if (strcasecmp(fields[0], "city") == 0 &&
            strcasecmp(fields[1], "lat") == 0) {
            continue;
        }

        if (fields[0][0] == '\0' || fields[3][0] == '\0' || fields[4][0] == '\0') {
            fprintf(stderr, "dotradio: %s:%d: empty city, station, or URL\n",
                    path, line_number);
            continue;
        }

        char *end = NULL;
        errno = 0;
        float lat = strtof(fields[1], &end);
        if (end == fields[1] || *end != '\0' || errno == ERANGE ||
            !isfinite(lat) || lat < -90.0f || lat > 90.0f) {
            fprintf(stderr, "dotradio: %s:%d: invalid latitude '%s'\n",
                    path, line_number, fields[1]);
            continue;
        }

        errno = 0;
        float lon = strtof(fields[2], &end);
        if (end == fields[2] || *end != '\0' || errno == ERANGE ||
            !isfinite(lon) || lon < -180.0f || lon > 180.0f) {
            fprintf(stderr, "dotradio: %s:%d: invalid longitude '%s'\n",
                    path, line_number, fields[2]);
            continue;
        }

        if (!marker_add_station(fields[0], lat, lon, fields[3], fields[4])) {
            fprintf(stderr, "dotradio: out of memory while reading %s\n", path);
            break;
        }

        valid_rows++;
    }

    free(line);
    fclose(file);

    if (valid_rows == 0) {
        fprintf(stderr, "dotradio: %s: no valid station rows\n", path);
    }
    return valid_rows > 0;
}

static void clear_markers(void) {
    for (int i = 0; i < MARKER_COUNT; i++) {
        for (int j = 0; j < MARKERS[i].num_channels; j++) {
            free((void *)MARKERS[i].channels[j].name);
            free((void *)MARKERS[i].channels[j].url);
        }
        free(MARKERS[i].channels);
        free((void *)MARKERS[i].city);
    }

    free(MARKERS);
    MARKERS = NULL;
    MARKER_COUNT = 0;
    MARKER_CAPACITY = 0;
}

static bool try_load(const char *path) {
    clear_markers();
    if (load_stations_file(path)) return true;
    clear_markers();
    return false;
}

static bool try_load_config(void) {
    char path[PATH_MAX];
    const char *xdg = getenv("XDG_CONFIG_HOME");

    if (xdg && xdg[0] != '\0') {
        int written = snprintf(path, sizeof(path), "%s/dotradio/stations.csv", xdg);
        if (written <= 0 || (size_t)written >= sizeof(path)) return false;
    } else {
        const char *home = getenv("HOME");
        if (!home || home[0] == '\0') return false;
        int written = snprintf(path, sizeof(path),
                               "%s/.config/dotradio/stations.csv", home);
        if (written <= 0 || (size_t)written >= sizeof(path)) return false;
    }

    return try_load(path);
}

static bool try_load_next_to_executable(void) {
    char exe[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (len <= 0) return false;
    exe[len] = '\0';

    char *slash = strrchr(exe, '/');
    if (!slash) return false;
    *slash = '\0';

    char path[PATH_MAX];
    int written = snprintf(path, sizeof(path),
                           "%s/../share/dotradio/stations.csv", exe);
    if (written <= 0 || (size_t)written >= sizeof(path)) return false;

    return try_load(path);
}

static void load_default_stations(void) {
    for (size_t i = 0; i < sizeof(DEFAULT_STATIONS) / sizeof(DEFAULT_STATIONS[0]); i++) {
        const DefaultStation *station = &DEFAULT_STATIONS[i];
        if (!marker_add_station(station->city, station->lat, station->lon,
                                station->station, station->url)) {
            fprintf(stderr, "dotradio: out of memory loading built-in stations\n");
            return;
        }
    }
}

static bool load_stations_from_file_or_defaults(void) {
    bool loaded = false;
    const char *override = getenv("DOTRADIO_STATIONS");

    if (override && override[0] != '\0') {
        loaded = try_load(override);
        if (!loaded) {
            fprintf(stderr,
                    "dotradio: could not read DOTRADIO_STATIONS=%s; "
                    "using built-in stations\n",
                    override);
        }
    } else {
        loaded =
            try_load("stations.csv") ||
            try_load_config() ||
            try_load_next_to_executable() ||
            try_load("/usr/local/share/dotradio/stations.csv") ||
            try_load("/usr/share/dotradio/stations.csv");
    }

    if (!loaded) load_default_stations();
    return loaded;
}

void world_init(void) {
    clear_markers();
    load_stations_from_file_or_defaults();

    LAND_COUNT = 0;
    for (int row = 0; row < LAND_ROWS; row++) {
        float lat = -90.0f + ((float)row + 0.5f) * (180.0f / (float)LAND_ROWS);
        for (int col = 0; col < LAND_COLS; col++) {
            if (!land_mask_test(row, col)) continue;

            float lon = -180.0f + ((float)col + 0.5f) * (360.0f / (float)LAND_COLS);
            if (LAND_COUNT < MAX_LAND_PTS) {
                LAND_PTS[LAND_COUNT++] = geoToSphere(lat, lon);
            }
        }
    }
}

void world_shutdown(void) {
    clear_markers();
}

int world_remove_channel(int marker_idx, int channel_idx) {
    if (marker_idx < 0 || marker_idx >= MARKER_COUNT) return -1;
    RadioMarker *marker = &MARKERS[marker_idx];
    if (channel_idx < 0 || channel_idx >= marker->num_channels) return -1;

    /* Free the removed channel's strings. */
    free((void *)marker->channels[channel_idx].name);
    free((void *)marker->channels[channel_idx].url);

    /* Shift remaining channels down. */
    for (int j = channel_idx + 1; j < marker->num_channels; j++) {
        marker->channels[j - 1] = marker->channels[j];
    }
    marker->num_channels--;

    /* Adjust active_channel_index if needed. */
    if (marker->active_channel_index >= marker->num_channels) {
        marker->active_channel_index = marker->num_channels > 0
            ? marker->num_channels - 1 : 0;
    }

    /* If the marker has no channels left, remove the marker too. */
    int removed_marker = marker_idx;
    if (marker->num_channels == 0) {
        free((void *)marker->city);
        free(marker->channels);

        for (int j = marker_idx + 1; j < MARKER_COUNT; j++) {
            MARKERS[j - 1] = MARKERS[j];
        }
        MARKER_COUNT--;
        removed_marker = -1;
    }

    return removed_marker;
}

int world_reload_stations(void) {
    clear_markers();
    load_stations_from_file_or_defaults();
    return MARKER_COUNT > 0 ? 0 : -1;
}

int world_save_stations(const char *path) {
    FILE *file = fopen(path, "w");
    if (!file) return -1;

    for (int i = 0; i < MARKER_COUNT; i++) {
        RadioMarker *m = &MARKERS[i];
        for (int j = 0; j < m->num_channels; j++) {
            fprintf(file, "%s,%.4f,%.4f,%s,%s\n",
                    m->city, m->lat, m->lon,
                    m->channels[j].name, m->channels[j].url);
        }
    }

    if (fclose(file) != 0) return -1;
    return 0;
}

int world_append_station(const char *city, float lat, float lon,
                         const char *station, const char *url) {
    FILE *file = fopen("stations.csv", "a");
    if (!file) return -1;

    int rc = fprintf(file, "%s,%.4f,%.4f,%s,%s\n",
                     city, lat, lon, station, url);
    if (rc < 0) {
        fclose(file);
        return -1;
    }

    if (fclose(file) != 0) return -1;

    /* Reload so the new station appears immediately. */
    return world_reload_stations();
}

int world_city_count(void) {
    return (int)(sizeof(CITIES) / sizeof(CITIES[0]));
}

const City *world_city_list(int index) {
    int count = (int)(sizeof(CITIES) / sizeof(CITIES[0]));
    if (index < 0 || index >= count) return NULL;
    return &CITIES[index];
}

const Vector3D *world_get_land_points(int *out_land_count) {
    if (out_land_count) *out_land_count = LAND_COUNT;
    return LAND_PTS;
}

RadioMarker *world_get_markers(int *out_marker_count) {
    if (out_marker_count) *out_marker_count = MARKER_COUNT;
    return MARKERS;
}
