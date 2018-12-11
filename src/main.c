#include "bms.h"
#include "graphics.h"
#include "input.h"
#include "mixer.h"
#include "play.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "tictoc/tictoc.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("No BMS file specified!\n");
		return 0;
	}

	if (!Graphics_init()) {
		return 0;
	}

	if (!Mixer_init(44100, 256)) {
		return 0;
	}

	if (!Input_init()) {
		return 0;
	}

	Play_init(argv[1]);

	TicTocTimer clock = tic();
	double dt = 0.0;

	int running = 1;

	while (running) {
		dt = toc(&clock);

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

		Play_update(dt);
		Graphics_clear();
		Play_draw();
		Graphics_present();
	}

	return 1;
}
