#ifndef MIXER_H
#define MIXER_H

#include <stdlib.h>

#define MIXER_HALTED 0
#define MIXER_PLAYING 1
#define MIXER_PAUSED 2

int Mixer_init();
void Mixer_destroy();
int Mixer_load_file(const char* path, float** buffer, size_t* size);
int Mixer_add(float* data, size_t size);
void Mixer_play();
void Mixer_pause();
void Mixer_halt();

#endif
