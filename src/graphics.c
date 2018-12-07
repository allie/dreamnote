#include "graphics.h"

SDL_Window* window;
SDL_Renderer* renderer;

int Graphics_init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 0;
	}

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		printf("IMG_Init error: %s\n", IMG_GetError());
		return 0;
	}

	window = SDL_CreateWindow(
		GRAPHICS_WIN_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GRAPHICS_WIN_WIDTH,
		GRAPHICS_WIN_HEIGHT,
		SDL_WINDOW_OPENGL
	);

	if (window == NULL) {
		printf("Error creating window: %s\n", SDL_GetError());
		return 0;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == NULL) {
		printf("Error creating renderer: %s\n", SDL_GetError());
		return 0;
	}

	SDL_RenderSetLogicalSize(renderer, GRAPHICS_WIN_WIDTH, GRAPHICS_WIN_HEIGHT);

	return 1;
}

void Graphics_clear() {
	if (renderer == NULL)
		return;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
}

void Graphics_present() {
	if (renderer == NULL)
		return;

	SDL_RenderPresent(renderer);
}

void Graphics_destroy() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}
