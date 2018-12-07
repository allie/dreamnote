#include "bms.h"
#include "graphics.h"
#include "mixer.h"
#include "play.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include "tictoc/tictoc.h"

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("No BMS file specified!\n");
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		printf("IMG_Init error: %s\n", IMG_GetError());
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

	Play_init(argv[1]);

	TicTocTimer clock = tic();
	double dt = 0.0;

	int running = 1;

	while (running) {
		dt = toc(&clock);

		SDL_Event event;
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_QUIT:
					running = 0;
					break;

				case SDL_KEYUP:
					switch (event.key.keysym.sym) {
						case SDLK_UP:
							Play_change_scroll_speed(100);
							break;

						case SDLK_DOWN:
							Play_change_scroll_speed(-100);
							break;

						default:
							break;
					}

				default:
					break;
			}
		}

		Play_update(dt);
		Graphics_clear();
		Play_draw();
		Graphics_present();
	}

	return 1;
}
