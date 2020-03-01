#include "graphics.h"
#include "log.h"
#include "play.h"

static SDL_Window* window;
static SDL_GLContext context;
static SDL_Thread* render_thread;
static int running, render_counter;

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
		Log_fatal("Error creating window: %s\n", SDL_GetError());
		return 0;
	}

	Log_debug("Successfully created window");

	render_thread = SDL_CreateThread(Graphics_thread, "Render", NULL);

	Log_debug("Successfully spawned render thread");

	return 1;
}

int Graphics_thread(void* data) {
	running = 1;

	context = SDL_GL_CreateContext(window);

	if (context == NULL) {
		Log_fatal("Error creating OpenGL context: %s\n", SDL_GetError());
		return 0;
	}

	Log_debug("Successfully created OpenGL context");

	// setting the projection matrix to match screen size is a temporary fix, move to using proper coords later
	glOrtho(0, GRAPHICS_WIN_WIDTH, GRAPHICS_WIN_HEIGHT, 0, -1, 1);
	if (SDL_GL_SetSwapInterval(-1) == -1) {
		SDL_GL_SetSwapInterval(1);
	}
	glClearColor(0.f, 0.f, 0.f, 1.f);

	Log_debug("Render thread beginning main loop");

	while (running) {
		Graphics_clear();
		Play_draw();
		Graphics_present();
		render_counter++;
	}
	return 1;
}

void Graphics_clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Graphics_present() {
	SDL_GL_SwapWindow(window);
}

void Graphics_destroy() {
	running = 0;
	SDL_WaitThread(render_thread, NULL);
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	Log_debug("Graphics successfully destroyed");
}

int Graphics_get_render_counter() {
	return render_counter;
}

void Graphics_reset_render_counter() {
	render_counter = 0;
}
