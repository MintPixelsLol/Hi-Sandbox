#ifndef AUDIOPLAY_H
#define AUDIOPLAY_H

#include <stdbool.h>

void audio_init(void);
bool audio_play(const char *path);
void audio_stop(void);

#endif
