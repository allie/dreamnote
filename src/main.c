#include "bms.h"
#include "graphics.h"
#include "input.h"
#include "mixer.h"
#include "play.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("No BMS file specified!\n");
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	Play_init(argv[1]);

	if (!Graphics_init()) {
		return 0;
	}

	if (!Mixer_init(44100, 256)) {
		return 0;
	}

	if (!Input_init()) {
		return 0;
	}

	struct timespec current_time;
	struct timespec new_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);

	int running = 1;
	int loop_counter = 0;

	while (running) {
		clock_gettime(CLOCK_MONOTONIC, &new_time);
		struct timespec dt = timespec_diff(current_time, new_time);

		Input_swap_state();

		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT:
					running = 0;
					break;

				case SDL_KEYDOWN:
					Input_key_pressed(event.key.keysym.scancode);
					break;

				case SDL_KEYUP:
					Input_key_released(event.key.keysym.scancode);
					break;

				case SDL_CONTROLLERBUTTONDOWN:
					Input_gamepad_pressed(event.cbutton.button);
					break;

				case SDL_CONTROLLERBUTTONUP:
					Input_gamepad_released(event.cbutton.button);
					break;

				default:
					break;
			}
		}

		Input_write_state();

		Play_update(dt.tv_nsec);

		memcpy(&current_time, &new_time, sizeof(struct timespec));

		printf("\rMain loop: %d, Render: %d", loop_counter++, Graphics_get_render_counter());
	}

	Graphics_destroy();
	SDL_Quit();

	return 1;
}
