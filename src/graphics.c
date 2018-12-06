#include "graphics.h"

static SDL_Window* window;
static SDL_Renderer* renderer;

int Graphics_init() {
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

SDL_Renderer* Graphics_get_renderer() {
	return renderer;
}

SDL_Window* Graphics_get_window() {
	return window;
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
