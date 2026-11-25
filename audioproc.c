#define _POSIX_C_SOURCE 200809L
#include "audioproc.h"
#include <dlfcn.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Minimal libmpv ABI declarations.
 *
 * We load libmpv at runtime with dlopen() so the program still builds and runs
 * (using the ffplay fallback) on systems without the libmpv development files.
 */
typedef struct mpv_handle mpv_handle;

typedef enum {
    MPV_EVENT_NONE = 0,
    MPV_EVENT_SHUTDOWN = 1,
    MPV_EVENT_END_FILE = 7
} mpv_event_id;

typedef struct {
    mpv_event_id event_id;
    int error;
    uint64_t reply_userdata;
    void *data;
} mpv_event;

typedef mpv_handle *(*mpv_create_fn)(void);
typedef int (*mpv_initialize_fn)(mpv_handle *);
typedef void (*mpv_terminate_destroy_fn)(mpv_handle *);
typedef int (*mpv_set_option_string_fn)(mpv_handle *, const char *, const char *);
typedef int (*mpv_command_fn)(mpv_handle *, const char **);
typedef mpv_event *(*mpv_wait_event_fn)(mpv_handle *, double);

typedef struct {
    void *lib;
    mpv_create_fn create;
    mpv_initialize_fn initialize;
    mpv_terminate_destroy_fn terminate_destroy;
    mpv_set_option_string_fn set_option_string;
    mpv_command_fn command;
    mpv_wait_event_fn wait_event;
} MpvApi;

static MpvApi g_mpv;
static bool g_mpv_tried = false;

static bool load_symbol(void *lib, const char *name, void **out) {
    *out = dlsym(lib, name);
    return *out != NULL;
}

static bool mpv_api_load(void) {
    if (g_mpv_tried) return g_mpv.lib != NULL;
    g_mpv_tried = true;

    /* Useful for testing the fallback path without removing libmpv. */
    const char *disable = getenv("DOTRADIO_NO_MPV");
    if (disable && disable[0] != '\0' && strcmp(disable, "0") != 0) {
        return false;
    }

    static const char *names[] = {
        "libmpv.so.2",
        "libmpv.so.1",
        "libmpv.so"
    };

    for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++) {
        void *lib = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
        if (!lib) continue;

        MpvApi api;
        memset(&api, 0, sizeof(api));
        api.lib = lib;

        bool ok = true;
        ok = ok && load_symbol(lib, "mpv_create", (void **)&api.create);
        ok = ok && load_symbol(lib, "mpv_initialize", (void **)&api.initialize);
        ok = ok && load_symbol(lib, "mpv_terminate_destroy", (void **)&api.terminate_destroy);
        ok = ok && load_symbol(lib, "mpv_set_option_string", (void **)&api.set_option_string);
        ok = ok && load_symbol(lib, "mpv_command", (void **)&api.command);
        ok = ok && load_symbol(lib, "mpv_wait_event", (void **)&api.wait_event);

        if (ok) {
            g_mpv = api;
            return true;
        }
        dlclose(lib);
    }

    return false;
}

static void audio_play_ffplay(AudioProc *ap, const char *url) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child process: replace image with ffplay */
        execlp("ffplay", "ffplay",
               "-nodisp",
               "-loglevel", "quiet",
               "-autoexit",
               url,
               (char *)NULL);
        _exit(127);
    }

    ap->pid = pid;
    ap->playing = true;
}

void audio_init(AudioProc *ap) {
    if (!ap) return;
    memset(ap, 0, sizeof(*ap));
    ap->pid = -1;

    if (!mpv_api_load()) return;

    mpv_handle *ctx = g_mpv.create();
    if (!ctx) return;

    g_mpv.set_option_string(ctx, "idle", "yes");
    g_mpv.set_option_string(ctx, "video", "no");
    g_mpv.set_option_string(ctx, "audio-display", "no");
    g_mpv.set_option_string(ctx, "terminal", "no");
    g_mpv.set_option_string(ctx, "input-default-bindings", "no");
    g_mpv.set_option_string(ctx, "input-vo-keyboard", "no");
    g_mpv.set_option_string(ctx, "force-window", "no");
    g_mpv.set_option_string(ctx, "msg-level", "all=no");

    if (g_mpv.initialize(ctx) < 0) {
        g_mpv.terminate_destroy(ctx);
        return;
    }

    ap->mpv = ctx;
    ap->using_mpv = true;
}

void audio_poll(AudioProc *ap) {
    if (!ap) return;

    if (ap->using_mpv && ap->mpv) {
        for (;;) {
            mpv_event *ev = g_mpv.wait_event((mpv_handle *)ap->mpv, 0.0);
            if (!ev || ev->event_id == MPV_EVENT_NONE) break;
            if (ev->event_id == MPV_EVENT_END_FILE || ev->event_id == MPV_EVENT_SHUTDOWN) {
                ap->playing = false;
            }
        }
        return;
    }

    if (ap->pid <= 0) return;

    int status = 0;
    pid_t res = waitpid(ap->pid, &status, WNOHANG);
    if (res == ap->pid || res < 0) {
        ap->pid = -1;
        ap->playing = false;
    }
}

void audio_stop(AudioProc *ap) {
    if (!ap) return;

    if (ap->using_mpv && ap->mpv) {
        const char *cmd[] = { "stop", NULL };
        g_mpv.command((mpv_handle *)ap->mpv, cmd);
        ap->playing = false;
        return;
    }

    if (ap->pid > 0) {
        int status = 0;
        pid_t res = waitpid(ap->pid, &status, WNOHANG);

        if (res == 0) {
            kill(ap->pid, SIGTERM);

            /* Give ffplay a short chance to exit cleanly, then force it. */
            for (int i = 0; i < 50; i++) {
                res = waitpid(ap->pid, &status, WNOHANG);
                if (res == ap->pid || res < 0) break;

                struct timespec ts = { 0, 10 * 1000 * 1000L };
                nanosleep(&ts, NULL);
            }

            if (res == 0) {
                kill(ap->pid, SIGKILL);
                waitpid(ap->pid, &status, 0);
            }
        }

        ap->pid = -1;
    }

    ap->playing = false;
}

void audio_shutdown(AudioProc *ap) {
    if (!ap) return;

    audio_stop(ap);

    if (ap->using_mpv && ap->mpv) {
        g_mpv.terminate_destroy((mpv_handle *)ap->mpv);
        ap->mpv = NULL;
        ap->using_mpv = false;
    }
}

bool audio_play_url(AudioProc *ap, const char *url) {
    if (!ap || !url || url[0] == '\0') return false;

    audio_stop(ap);

    if (ap->using_mpv && ap->mpv) {
        const char *cmd[] = { "loadfile", url, "replace", NULL };
        if (g_mpv.command((mpv_handle *)ap->mpv, cmd) >= 0) {
            ap->playing = true;
            return true;
        }

        /* libmpv rejected the command; use ffplay from now on. */
        fprintf(stderr, "libmpv could not load the stream; falling back to ffplay\n");
        g_mpv.terminate_destroy((mpv_handle *)ap->mpv);
        ap->mpv = NULL;
        ap->using_mpv = false;
    }

    audio_play_ffplay(ap, url);
    return ap->pid > 0;
}
