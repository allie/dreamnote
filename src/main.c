#include "log.h"
#include "bms.h"
#include "graphics.h"
#include "input.h"
#include "mixer.h"
#include "play.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

static const int LOOP_RATE_HZ = 250;
static const int LOOP_TIME_MS = 1000 / LOOP_RATE_HZ;

int main(int argc, char* argv[]) {
	Log_start("dreamnote.log", LOG_DEBUG, 1);

	if (argc < 2) {
		Log_fatal("No BMS file specified!");
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		Log_fatal("SDL_Init error: %s", SDL_GetError());
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

	struct timespec loop_last;
	struct timespec loop_start;
	struct timespec loop_end;
	struct timespec loop_target;
	clock_gettime(CLOCK_MONOTONIC, &loop_last);

	int running = 1;
	int loop_counter = 0;
	int sleep_time = 0;
	int rate_timer = loop_last.tv_sec;

	Log_debug("Beginning main thread event loop");

	while (running) {
		clock_gettime(CLOCK_MONOTONIC, &loop_start);
		struct timespec dt = timespec_diff(loop_last, loop_start);
		loop_target = timespec_add_ns(loop_start, LOOP_TIME_MS * 1E6);

		Input_swap_state();

		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT:
					Log_info("Initiating shutdown");
					running = 0;
					break;

				case SDL_KEYDOWN:
					if (event.key.repeat == 0) {
						if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
							Play_change_scroll_speed(100);
						} else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
							Play_change_scroll_speed(-100);
						} else {
							Input_key_pressed(event.key.keysym.scancode);
						}
					}
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

		memcpy(&loop_last, &loop_start, sizeof(struct timespec));

		/* keep track of and print thread loop rates
		loop_counter++;
		if (loop_start.tv_sec >= rate_timer) {
			printf("\rmain rate: %f, render rate: %f", loop_counter / 3.0, Graphics_get_render_counter() / 3.0);
			rate_timer += 3;
			loop_counter = 0;
			Graphics_reset_render_counter();
			fflush(stdout);
		}
		*/

		clock_gettime(CLOCK_MONOTONIC, &loop_end);
		dt = timespec_diff(loop_end, loop_target);
		sleep_time = dt.tv_nsec / 1E6;
		if (sleep_time > 0 && sleep_time <= LOOP_TIME_MS) {
			SDL_Delay(sleep_time);
			do {
				clock_gettime(CLOCK_MONOTONIC, &loop_end);
				dt = timespec_diff(loop_end, loop_target);
			} while (dt.tv_sec >= 0);
		}
	}

	Log_debug("Ended main thread event loop");

	// destroy input
	// destroy mixer
	Graphics_destroy();
	Play_destroy();
	SDL_Quit();
	Log_destroy();

	return 1;
}
