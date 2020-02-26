#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_thread.h>

#define GRAPHICS_WIN_TITLE "dreamnote"
#define GRAPHICS_WIN_WIDTH 800
#define GRAPHICS_WIN_HEIGHT 600

int Graphics_init();
int Graphics_thread();
SDL_Renderer* Graphics_get_renderer();
SDL_Window* Graphics_get_window();
void Graphics_clear();
void Graphics_present();
void Graphics_destroy();
int Graphics_get_render_counter();

#endif
