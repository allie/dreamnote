// This is not intended to be used for the actual game, but
// just a playground for testing gameplay.

#include "play.h"
#include "bms.h"
#include "graphics.h"
#include "util.h"
#include "animation.h"
#include "input.h"

#include <SDL2/SDL.h>

static BMS* bms;
static double measure_height = GRAPHICS_WIN_HEIGHT * 2;
static double lane_width = 80.0;
static double judge_line = GRAPHICS_WIN_HEIGHT - 100.0;
static Measure** render_objects;
static Animation* bombs[8];
static SDL_Rect bomb_positions[8];

extern SDL_Renderer* renderer;

void Play_init(char* path) {
	bms = BMS_load(path);

	if (bms == NULL) {
		printf("Error trying to play %s\n.", path);
		return;
	}

	render_objects = BMS_get_renderable_objects(bms);

	// Load bomb animations
	for (int i = 0; i < 8; i++) {
		bombs[i] = Animation_load_from_file("assets/animations/bomb.png", 13, 128, 1/60.0, 0, 1);
		bomb_positions[i].x = i * lane_width + (lane_width / 2.0) - (bombs[i]->frame_width / 2.0);
		bomb_positions[i].y = judge_line - 4 - bombs[i]->height / 2.0;
	}
}

void Play_change_scroll_speed(int diff) {
	measure_height += diff;
}

void Play_update(long dt) {
	BMS_step(bms, dt);
	Animation_update_all(dt);
}

void Play_draw() {
	SDL_Rect rect;

	// Draw lane separations
	for (int i = 0; i <= (bms->format == FORMAT_PMS ? 9 : 8); i++) {
		rect.x = i * lane_width - 1;
		rect.y = 0;
		rect.w = 2;
		rect.h = judge_line;
		SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
		SDL_RenderFillRect(renderer, &rect);
	}

	// Draw the judge line
	rect.x = 0;
	rect.y = judge_line - 8;
	rect.w = GRAPHICS_WIN_WIDTH;
	rect.h = 8;
	SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);
	SDL_RenderFillRect(renderer, &rect);

	for (int i = 0; i < bms->total_measures; i++) {
		Measure* measure = render_objects[i];

		double y = judge_line - ((i + 1) * measure_height) + (bms->current_measure + bms->current_measure_part) * measure_height;
		double x = 0.0;

		// Draw the bar line
		if (y - 1 <= judge_line) {
			rect.x = 0;
			rect.y = y - 1;
			rect.w = (bms->format == FORMAT_PMS ? 9 : 8) * lane_width;
			rect.h = 2;
			SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
			SDL_RenderFillRect(renderer, &rect);
		}

		// Draw notes
		for (int j = 0; j < measure->channel_count; j++) {
			Channel* channel = measure->channels[j];

			for (int k = 0; k < channel->object_count; k++) {
				Object* object = channel->objects[k];

				rect.x = x + object->lane * lane_width;
				rect.y = y + (object->ypos * measure_height) - 8;
				rect.w = lane_width;
				rect.h = 8;

				if (rect.y >= -8 && rect.y <= judge_line - 8) {
					switch (bms->format) {
						case FORMAT_BMS:
						case FORMAT_BME: {
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
							break;
						}

						case FORMAT_PMS:{
							switch (object->lane) {
								case 0:
								case 8:
									SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
									break;

								case 1:
								case 7:
									SDL_SetRenderDrawColor(renderer, 255, 217, 0, 255);
									break;

								case 2:
								case 6:
									SDL_SetRenderDrawColor(renderer, 38, 255, 0, 255);
									break;

								case 3:
								case 5:
									SDL_SetRenderDrawColor(renderer, 0, 238, 255, 255);
									break;

								case 4:
									SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
									break;
							}
							break;
						}
					}

					SDL_RenderFillRect(renderer, &rect);
				}

				if (rect.y >= judge_line - 8 && rect.y <= judge_line + 8) {
					// Animation_stop(bombs[object->lane]);
					// Animation_play(bombs[object->lane]);
				}
			}
		}
	}

	for (int i = 0; i < 8; i++) {
		// Animation_draw(bombs[i], bomb_positions[i].x, bomb_positions[i].y);
	}
}
