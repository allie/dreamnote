#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#define NUM_BUTTONS 20

// Bindings for all 20 buttons
// Example:
// scancodes[0] = SDL_SCANCODE_Z; | Button 1 = scancode for the Z key
// gamepad[0] = SDL_CONTROLLER_BUTTON_A | Button 1 = A button on the gamepad
typedef struct {
	SDL_Scancode scancodes[NUM_BUTTONS];
	SDL_GameControllerButton gamepad[NUM_BUTTONS];
} InputMapping;

// The composite state of all scancode and gamepad bindings
typedef struct {
	int buttons[NUM_BUTTONS];
	int pressed[NUM_BUTTONS];
	int released[NUM_BUTTONS];
	int scancodes[NUM_BUTTONS];
	int gamepad[NUM_BUTTONS];
} InputState;

int Input_init();
void Input_set_default_bindings();
void Input_key_pressed(SDL_Scancode key);
void Input_key_released(SDL_Scancode key);
void Input_gamepad_pressed(SDL_GameControllerButton button);
void Input_gamepad_released(SDL_GameControllerButton button);
void Input_swap_state();
void Input_write_state();
int Input_is_down(int button);
int Input_was_pressed(int button);
int Input_was_released(int button);

#endif
