#include "bms.h"
#include "graphics.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "tictoc/tictoc.h"

int main() {
	BMS* bms = BMS_load("Faulty Sparkle/faultyANOTHER.bms");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	if (!Graphics_init()) {
		return 0;
	}

	TicTocTimer clock = tic();
	double dt = 0.0;

	int running = 1;

	while (running) {
		dt = toc(&clock);

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				running = 0;
			}
		}

		BMS_step(bms, dt);

		Graphics_clear();
		Graphics_present();
	}

	BMS_free(bms);

	return 1;
}
