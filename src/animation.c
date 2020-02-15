#include "animation.h"
#include "graphics.h"
#include "util.h"

static Animation** animations = NULL;
static int animation_count = 0;

extern SDL_Renderer* renderer;

// Load an animation sheet from a PNG file
Animation* Animation_load_from_file(const char* path, int total_frames, int frame_width, double frame_duration, int loop, int hide_when_stopped) {
	Animation* animation = calloc(1, sizeof(Animation));

	// Load the image
	animation->texture = IMG_LoadTexture(renderer, path);

	// Return NULL if the image doesn't load
	if (animation->texture == NULL) {
		free(animation);
		return NULL;
	}

	// Initialize all fields
	animation->total_frames = total_frames;
	animation->frame_width = frame_width;
	animation->frame_duration = frame_duration;
	animation->loop = loop;
	animation->hide_when_stopped = hide_when_stopped;
	animation->current_frame = 0;
	animation->frame_timer = 0;
	animation->playing = 0;
	SDL_QueryTexture(animation->texture, NULL, NULL, NULL, &animation->height);

	// Add the animation to the global animation array
	animations = recalloc(animations, sizeof(Animation*), animation_count, animation_count + 1);
	animations[animation_count] = animation;
	animation_count++;

	return animation;
}

// Play the animation
void Animation_play(Animation* animation) {
	if (animation == NULL) {
		return;
	}

	animation->playing = 1;
}

// Pause the animation
void Animation_pause(Animation* animation) {
	if (animation == NULL) {
		return;
	}

	animation->playing = 0;
}

// Stop the animation and reset the frame to 0
void Animation_stop(Animation* animation) {
	if (animation == NULL) {
		return;
	}

	animation->playing = 0;
	animation->current_frame = 0;
	animation->frame_timer = 0;
}

// Update all animations currently loaded
void Animation_update_all(long dt) {
	for (int i = 0; i < animation_count; i++) {
		Animation_update(animations[i], dt);
	}
}

// Update a particular animation
void Animation_update(Animation* animation, long dt) {
	// Do nothing if animation isn't playing
	if (!animation->playing) {
		return;
	}
	// Advance the timer
	animation->frame_timer += dt;

	// If the frame duration has passed, advance the frame and reset the timer
	while (animation->frame_timer >= (animation->frame_duration * 1E9)) {
		animation->frame_timer -= animation->frame_duration;
		animation->current_frame = ++animation->current_frame % animation->total_frames;

		// If frame wrapped around and loop is disabled, stop the animation
		if (!animation->loop && animation->current_frame == 0) {
			animation->playing = 0;
			animation->current_frame = 0;
			animation->frame_timer = 0;
		}
	}
}

// Draw the animation to the screen at the given coordinates
void Animation_draw(Animation* animation, int x, int y) {
	//  If set to hide when stopped and the animation is stopped, don't draw it
	if (!animation->playing && animation->hide_when_stopped) {
		return;
	}

	// Source rectangle
	SDL_Rect src;
	src.x = animation->current_frame * animation->frame_width;
	src.y = 0;
	src.w = animation->frame_width;
	src.h = animation->height;

	// Destination rectangle
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = animation->frame_width;
	dst.h = animation->height;

	// Draw to the screen
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderCopy(renderer, animation->texture, &src, &dst);
}

