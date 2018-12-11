#include "input.h"

static InputMapping bindings;
static InputState last_state;
static InputState current_state;

// Find the button index that the given scancode is bound to
// Returns -1 if no binding is found
static inline int get_button_for_scancode(SDL_Scancode key) {
	for (int i = 0; i < NUM_BUTTONS; i++) {
		if (bindings.scancodes[i] == key) {
			return i;
		}
	}

	return -1;
}

// Find the button index that the given gamepad button is bound to
// Returns -1 if no binding is found
static inline int get_button_for_gamepad(SDL_GameControllerButton button) {
	for (int i = 0; i < NUM_BUTTONS; i++) {
		if (bindings.gamepad[i] == button) {
			return i;
		}
	}

	return -1;
}

// Initialize the input module
int Input_init() {
	for (int i = 0; i < NUM_BUTTONS; i++) {
		bindings.scancodes[i] = 0;
		bindings.gamepad[i] = 0;
		last_state.buttons[i] = 0;
		last_state.pressed[i] = 0;
		last_state.released[i] = 0;
		last_state.scancodes[i] = 0;
		last_state.gamepad[i] = 0;
		current_state.buttons[i] = 0;
		current_state.pressed[i] = 0;
		current_state.released[i] = 0;
		current_state.scancodes[i] = 0;
		current_state.gamepad[i] = 0;
	}

	Input_set_default_bindings();

	return 1;
}

// Set default bindings to be suitable for IIDX SP / pop'n style charts
//        S D F G         2 4 6 8
// Shift Z X C V B    TT 1 3 5 7 9
void Input_set_default_bindings() {
	bindings.scancodes[0] = SDL_SCANCODE_Z;
	bindings.scancodes[1] = SDL_SCANCODE_S;
	bindings.scancodes[2] = SDL_SCANCODE_X;
	bindings.scancodes[3] = SDL_SCANCODE_D;
	bindings.scancodes[4] = SDL_SCANCODE_C;
	bindings.scancodes[5] = SDL_SCANCODE_F;
	bindings.scancodes[6] = SDL_SCANCODE_V;
	bindings.scancodes[7] = SDL_SCANCODE_G;
	bindings.scancodes[8] = SDL_SCANCODE_B;
}

// Handle a key being pressed
void Input_key_pressed(SDL_Scancode key) {
	int index = get_button_for_scancode(key);

	if (index == -1) {
		return;
	}

	current_state.scancodes[index] = 1;
}

// Handle a key being released
void Input_key_released(SDL_Scancode key) {
	int index = get_button_for_scancode(key);

	if (index == -1) {
		return;
	}

	current_state.scancodes[index] = 0;
}

// Handle a button being pressed
void Input_gamepad_pressed(SDL_GameControllerButton button) {
	int index = get_button_for_gamepad(button);

	if (index == -1) {
		return;
	}

	current_state.gamepad[index] = 1;
}

// Handle a button being released
void Input_gamepad_released(SDL_GameControllerButton button) {
	int index = get_button_for_gamepad(button);

	if (index == -1) {
		return;
	}

	current_state.gamepad[index] = 0;
}

// Copy the current input state to the last input state in preparation for polling new events
void Input_swap_state() {
	for (int i = 0; i < NUM_BUTTONS; i++) {
		last_state.buttons[i] = current_state.buttons[i];
		last_state.pressed[i] = current_state.pressed[i];
		last_state.released[i] = current_state.released[i];
		last_state.scancodes[i] = current_state.scancodes[i];
		last_state.gamepad[i] = current_state.gamepad[i];
		current_state.pressed[i] = 0;
		current_state.released[i] = 0;
	}
}

// Write the new input state by compositing keyboard and gamepad states
void Input_write_state() {
	for (int i = 0; i < NUM_BUTTONS; i++) {
		current_state.buttons[i] = current_state.scancodes[i] || current_state.gamepad[i];
		current_state.pressed[i] = current_state.buttons[i] && !last_state.buttons[i];
		current_state.released[i] = !current_state.buttons[i] && last_state.buttons[i];
	}
}

// Determine whether a particular button index is down
int Input_is_down(int button) {
	if (button < 0 || button >= NUM_BUTTONS) {
		return 0;
	}

	return current_state.buttons[button];
}

// Determine whether a particular button index was pressed between the last update and now
int Input_was_pressed(int button) {
	if (button < 0 || button >= NUM_BUTTONS) {
		return 0;
	}

	return current_state.pressed[button];
}

// Determine whether a particular button index was released between the last update and now
int Input_was_released(int button) {
	if (button < 0 || button >= NUM_BUTTONS) {
		return 0;
	}

	return current_state.released[button];
}
