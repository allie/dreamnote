#include "bms.h"
#include "graphics.h"
#include "mixer.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include "tictoc/tictoc.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("No BMS file specified!\n");
		return 0;
	}

	BMS* bms = BMS_load(argv[1]);

	if (bms == NULL) {
		printf("Error trying to play %s\n.", argv[1]);
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	if (!Graphics_init()) {
		return 0;
	}

	PaError error = Pa_Initialize();
	if (error != paNoError) {
		printf("PortAudio error: %s\n", Pa_GetErrorText(error));
		return 0;
	}

	if (!Mixer_init(44100, 256)) {
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
