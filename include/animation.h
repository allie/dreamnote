#ifndef ANIMATION_H
#define ANIMATION_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef struct {
	SDL_Texture* texture;
	int total_frames;
	int current_frame;
	int frame_width;
	int height;
	double frame_duration;
	long frame_timer;
	int loop;
	int playing;
	int hide_when_stopped;
} Animation;

Animation* Animation_load_from_file(const char* path, int total_frames, int frame_width, double frame_duration, int loop, int hide_when_stopped);
void Animation_play(Animation* animation);
void Animation_pause(Animation* animation);
void Animation_stop(Animation* animation);
void Animation_update_all(long dt);
void Animation_update(Animation* animation, long dt);
void Animation_draw(Animation* animation, int x, int y);

#endif
