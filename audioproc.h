#ifndef AUDIOPROC_H
#define AUDIOPROC_H

#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    void *mpv;          /* mpv_handle* when the libmpv backend is active */
    pid_t pid;          /* ffplay child when the fallback backend is active */
    bool using_mpv;
    bool playing;
} AudioProc;

void audio_init(AudioProc *ap);
void audio_poll(AudioProc *ap);
void audio_stop(AudioProc *ap);
void audio_shutdown(AudioProc *ap);
bool audio_play_url(AudioProc *ap, const char *url);

#endif
