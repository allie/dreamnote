#include "play.h"
#include "bms.h"
#include "graphics.h"
#include "util.h"

#include <SDL2/SDL.h>

static SDL_Renderer* renderer;
static BMS* bms;
static double measure_height = GRAPHICS_WIN_HEIGHT;
static double lane_width = 80.0;
static Measure** render_objects;

void Play_init(char* path) {
	renderer = Graphics_get_renderer();

	bms = BMS_load(path);

	if (bms == NULL) {
		printf("Error trying to play %s\n.", path);
		return;
	}

	render_objects = BMS_get_renderable_objects(bms);
}

void Play_change_scroll_speed(int diff) {
	measure_height += diff;
}

void Play_update(double dt) {
	BMS_step(bms, dt);
}

void Play_draw() {
	for (int i = 0; i < bms->total_measures; i++) {
		Measure* measure = render_objects[i];

		double y = GRAPHICS_WIN_HEIGHT - (i * measure_height) + (bms->current_measure + bms->current_measure_part) * measure_height;
		double x = 0.0;

		for (int j = 0; j < measure->channel_count; j++) {
			Channel* channel = measure->channels[j];

			for (int k = 0; k < channel->object_count; k++) {
				Object* object = channel->objects[k];

				SDL_Rect rect;
				rect.x = x + object->lane * lane_width;
				rect.y = y + (object->ypos * measure_height) - 8;
				rect.w = lane_width;
				rect.h = 8;

				switch (object->lane) {
					case 0:
						SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
						break;

					case 1:
					case 3:
					case 5:
					case 7:
						SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
						break;

					case 2:
					case 4:
					case 6:
						SDL_SetRenderDrawColor(renderer, 66, 134, 244, 255);
						break;
				}
				SDL_RenderFillRect(renderer, &rect);
			}
		}
	}
}
