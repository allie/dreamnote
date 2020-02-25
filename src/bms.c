#include "bms.h"
#include "mixer.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

// Determines whether a channel number is a WAV channel or not
static inline int is_wav_channel(int channel) {
	return channel != 0 && // Retired channel
		(channel < 2 || channel > 14) && // Settings channels
		channel != 36 && // Reserved channel
		channel != 72 && // Reserved channel
		channel != 108 && // Reserved channel
		channel != 144 && // Reserved channel
		channel != 180 && // Reserved channel
		channel != 216 && // Reserved channel
		(channel < 252 || channel > 330) && // Reserved channels
		channel != 331 && // BGM volume
		channel != 332 && // KEY volume
		channel != 333 && // TEXT
		(channel < 360 || channel > 366); // More settings channels
}

// Determines whether a channel number is a visible channel or not
static inline int is_visible_channel(int channel) {
	return (channel >= 37 && channel <= 71) || // 1P visible
		(channel >= 73 && channel <= 107) || // 2P visible
		(channel >= 181 && channel <= 215) || // 1P long note
		(channel >= 217 && channel <= 251); // 2P long note
}

// #PLAYER x
static int parse_player(BMS* bms, char* command) {
	if (stristr(command, "#PLAYER")) {
		command += strlen("#PLAYER");
		long value = strtol(command, NULL, 10);

		if (value >= PLAY_SINGLE && value <= PLAY_BATTLE) {
			bms->play_type = (int)value;
			return 1;
		}
	}

	return 0;
}

// #GENRE x
// Due to a typo at some point in BMS history, GENLE is also accepted
static int parse_genre(BMS* bms, char* command) {
	if (stristr(command, "#GENRE") || stristr(command, "#GENLE")) {
		command += strlen("#GENRE");
		bms->genre = strdup(command);
		return 1;
	}

	return 0;
}

// #ARTIST x
static int parse_artist(BMS* bms, char* command) {
	if (stristr(command, "#ARTIST")) {
		command += strlen("#ARTIST");
		bms->artist = strdup(command);
		return 1;
	}

	return 0;
}

// #SUBARTIST x
static int parse_subartist(BMS* bms, char* command) {
	if (stristr(command, "#SUBARTIST")) {
		command += strlen("#SUBARTIST");

		// Resize the defs array
		int old_count = bms->subartist_count;
		bms->subartist_count++;
		bms->subartists = recalloc(bms->subartists, sizeof(char*), old_count, bms->subartist_count);

		// Create a new entry in the defs array
		bms->subartists[old_count] = strdup(command);
		return 1;
	}

	return 0;
}

// #MAKER x
static int parse_maker(BMS* bms, char* command) {
	if (stristr(command, "#MAKER")) {
		command += strlen("#MAKER");
		bms->maker = strdup(command);
		return 1;
	}

	return 0;
}

// #TITLE x
static int parse_title(BMS* bms, char* command) {
	if (stristr(command, "#TITLE")) {
		command += strlen("#TITLE");
		bms->title = strdup(command);
		return 1;
	}

	return 0;
}

// #SUBTITLE x
static int parse_subtitle(BMS* bms, char* command) {
	if (stristr(command, "#SUBTITLE")) {
		command += strlen("#SUBTITLE");
		bms->subtitle = strdup(command);
		return 1;
	}

	return 0;
}

// #BPM x
// Not to be confused with #BPMxx yy
static int parse_bpm(BMS* bms, char* command) {
	if (stristr(command, "#BPM") && iswhitespace(command[4])) {
		command += strlen("#BPM");
		bms->init_bpm = strtod(command, NULL);
		return 1;
	}

	return 0;
}

// #RANK x
static int parse_rank(BMS* bms, char* command) {
	if (stristr(command, "#RANK")) {
		command += strlen("#RANK");
		long value = strtol(command, NULL, 10);
		bms->rank = (int)value;
		return 1;
	}

	return 0;
}

// #TOTAL x
static int parse_total(BMS* bms, char* command) {
	if (stristr(command, "#TOTAL")) {
		command += strlen("#TOTAL");
		bms->total = strtod(command, NULL);
		return 1;
	}

	return 0;
}

// #VOLWAV x
static int parse_volwav(BMS* bms, char* command) {
	if (stristr(command, "#VOLWAV")) {
		command += strlen("#VOLWAV");
		bms->volwav = strtod(command, NULL);
		return 1;
	}

	return 0;
}

// #WAVxx <filename>
static int parse_wav(BMS* bms, char* command) {
	if (stristr(command, "#WAV")) {
		command += strlen("#WAV");

		// Extract the base 36 ID (xx)
		char id_base36[] = {command[0], command[1], '\0'};
		long id = strtol(id_base36, NULL, 36);

		// Resize the defs array
		int old_count = bms->wav_def_count;
		bms->wav_def_count = id + 1;
		bms->wav_defs = recalloc(bms->wav_defs, sizeof(WavDef*), old_count, bms->wav_def_count);

		command += 2;
		trim(command);

		// Create a new entry in the defs array
		bms->wav_defs[id] = malloc(sizeof(WavDef));
		char file[1024];
		strcpy(file, bms->directory);
		strcat(file, "/");
		strcat(file, command);
		bms->wav_defs[id]->file = strdup(file);

		// Open the file
		float* buffer = NULL;
		size_t size = 0;
		
		if (!Mixer_load_file(bms->wav_defs[id]->file, &buffer, &size)) {
			printf("Could not open WAV%ld (%s).\n", id, command);
			return 0;
		}

		// Extract the data
		bms->wav_defs[id]->data = buffer;
		bms->wav_defs[id]->size = size;
		return 1;
	}

	return 0;
}

// #BMPxx <filename>
static int parse_bmp(BMS* bms, char* command) {
	if (stristr(command, "#BMP")) {
		command += strlen("#BMP");

		// Extract the base 36 ID (xx)
		char id_base36[] = {command[0], command[1], '\0'};
		long id = strtol(id_base36, NULL, 36);

		// Resize the defs array
		int old_count = bms->bmp_def_count;
		bms->bmp_def_count = id + 1;
		bms->bmp_defs = recalloc(bms->bmp_defs, sizeof(BmpDef*), old_count, bms->bmp_def_count);

		command += 2;

		// Create a new entry in the defs array
		bms->bmp_defs[id] = malloc(sizeof(BmpDef));
		char file[1024];
		strcpy(file, bms->directory);
		strcat(file, "/");
		strcat(file, command);
		bms->bmp_defs[id]->file = strdup(file);
		return 1;
	}

	return 0;
}

// #TEXTxx "<message>"
// #TEXTxx <message>
static int parse_text(BMS* bms, char* command) {
	if (stristr(command, "#TEXT")) {
		command += strlen("#TEXT");

		// Extract the base 36 ID (xx)
		char id_base36[] = {command[0], command[1], '\0'};
		long id = strtol(id_base36, NULL, 36);

		// Resize the defs array
		int old_count = bms->text_def_count;
		bms->text_def_count = id + 1;
		bms->text_defs = recalloc(bms->text_defs, sizeof(char*), old_count, bms->text_def_count);

		command += 2;

		// Strip quotes
		if (command[0] == '"') {
			command++;
			command[strlen(command) - 1] = '\0';
		}

		// Create a new entry in the defs array
		bms->text_defs[id] = strdup(command);
		return 1;
	}

	return 0;
}

// #COMMENT "<message>"
// #COMMENT <message>
static int parse_comment(BMS* bms, char* command) {
	if (stristr(command, "#COMMENT")) {
		command += strlen("#COMMENT");

		// Resize the defs array
		int old_count = bms->comment_count;
		bms->comment_count++;
		bms->comments = recalloc(bms->comments, sizeof(char*), old_count, bms->comment_count);

		// Strip quotes
		if (command[0] == '"') {
			command++;
			command[strlen(command) - 1] = '\0';
		}

		// Create a new entry in the defs array
		bms->comments[old_count] = strdup(command);
		return 1;
	}

	return 0;
}

// #BPMxx <new BPM>
static int parse_bpmex(BMS* bms, char* command) {
	if (stristr(command, "#BPM")) {
		command += strlen("#BPM");

		// Extract the base 36 ID (xx)
		char id_base36[] = {command[0], command[1], '\0'};
		long id = strtol(id_base36, NULL, 36);

		// Resize the defs array
		int old_count = bms->bpm_def_count;
		bms->bpm_def_count = id + 1;
		bms->bpm_defs = recalloc(bms->bpm_defs, sizeof(double*), old_count, bms->bpm_def_count);

		command += 2;

		// Create a new entry in the defs array
		bms->bpm_defs[id] = strtod(command, NULL);
		return 1;
	}

	return 0;
}

// #xxxyy:zz
static int parse_line(BMS* bms, char* command) {
	if (command[6] == ':') {
		// Extract the measure number (xxx)
		char measure_str[] = {command[1], command[2], command[3], '\0'};
		long measure_num = strtol(measure_str, NULL, 10);

		// Extract the channel number
		char channel_str[] = {command[4], command[5], '\0'};
		long channel_num = strtol(channel_str, NULL, 36);

		// Extract the message
		char* message = command + strlen("#xxxyy:");

		// Resize the measures array if need be
		int old_count = bms->measure_count;
		bms->measure_count = measure_num + 1;
		bms->measures = recalloc(bms->measures, sizeof(Measure*), old_count, bms->measure_count);

		// If the measure doesn't exist, create it
		if (bms->measures[measure_num] == NULL) {
			bms->measures[measure_num] = malloc(sizeof(Measure));
			bms->measures[measure_num]->channel_count = 0;
			bms->measures[measure_num]->channels = NULL;
			bms->measures[measure_num]->bgm_channel_count = 0;
			bms->measures[measure_num]->bgm_channels = NULL;
			bms->measures[measure_num]->metre = 1.0;
		}

		// Particular channel numbers are just used for changing settings
		switch (channel_num) {
			case CHANNEL_METRE:
				bms->measures[measure_num]->metre = strtod(message, NULL);
				return 1;

			// More cases will go here

			default:
				break;
		}

		// BGM channel can have nested channels of its own, so they are handled differently
		if (channel_num == CHANNEL_BGM) {
			// Incremement the BGM channel count and resize
			int bgm_channel_index = bms->measures[measure_num]->bgm_channel_count;
			bms->measures[measure_num]->bgm_channel_count++;
			bms->measures[measure_num]->bgm_channels = recalloc(
				bms->measures[measure_num]->bgm_channels,
				sizeof(Channel*),
				bgm_channel_index,
				bms->measures[measure_num]->bgm_channel_count
			);

			// Create a new BGM channel for this measure
			bms->measures[measure_num]->bgm_channels[bgm_channel_index] = malloc(sizeof(Channel));
			bms->measures[measure_num]->bgm_channels[bgm_channel_index]->objects = NULL;

			// Count the objects and allocate an array of objects for this channel
			bms->measures[measure_num]->bgm_channels[bgm_channel_index]->object_count = strlen(message) / 2;
			bms->measures[measure_num]->bgm_channels[bgm_channel_index]->objects = calloc(
				sizeof(Object*),
				bms->measures[measure_num]->bgm_channels[bgm_channel_index]->object_count
			);

			// Populate the object data with base 36 IDs
			for (int i = 0; i < bms->measures[measure_num]->bgm_channels[bgm_channel_index]->object_count; i++) {
				// Extract the ID (zz)
				char id_base36[] = {message[i * 2], message[i * 2 + 1], '\0'};
				long id = strtol(id_base36, NULL, 36);

				bms->measures[measure_num]->bgm_channels[bgm_channel_index]->objects[i] = malloc(sizeof(Object));
				bms->measures[measure_num]->bgm_channels[bgm_channel_index]->objects[i]->id = (int)id;
				bms->measures[measure_num]->bgm_channels[bgm_channel_index]->objects[i]->activated = 0;
			}
		}

		// All other channels
		else {
			// Resize the channels array if need be
			old_count = bms->measures[measure_num]->channel_count;
			bms->measures[measure_num]->channel_count = channel_num + 1;
			bms->measures[measure_num]->channels = recalloc(
				bms->measures[measure_num]->channels,
				sizeof(Channel*),
				old_count,
				bms->measures[measure_num]->channel_count
			);

			// If the channel doesn't exist, create it
			if (bms->measures[measure_num]->channels[channel_num] == NULL) {
				bms->measures[measure_num]->channels[channel_num] = malloc(sizeof(Channel));
				bms->measures[measure_num]->channels[channel_num]->objects = NULL;
			}

			// If an objects array already exists, free it so we can overwrite it
			if (bms->measures[measure_num]->channels[channel_num]->objects != NULL) {
				free(bms->measures[measure_num]->channels[channel_num]->objects);
				bms->measures[measure_num]->channels[channel_num]->objects = NULL;
			}

			// Count the objects and allocate an array of objects for this channel
			bms->measures[measure_num]->channels[channel_num]->object_count = strlen(message) / 2;
			bms->measures[measure_num]->channels[channel_num]->objects = calloc(
				sizeof(Object*),
				bms->measures[measure_num]->channels[channel_num]->object_count
			);

			// Populate the object data with base 36 IDs
			for (int i = 0; i < bms->measures[measure_num]->channels[channel_num]->object_count; i++) {
				// Extract the ID (zz)
				char id_base36[] = {message[i * 2], message[i * 2 + 1], '\0'};
				long id = strtol(id_base36, NULL, 36);

				bms->measures[measure_num]->channels[channel_num]->objects[i] = malloc(sizeof(Object));
				bms->measures[measure_num]->channels[channel_num]->objects[i]->id = (int)id;
				bms->measures[measure_num]->channels[channel_num]->objects[i]->visible = (int)id != 0;
				bms->measures[measure_num]->channels[channel_num]->objects[i]->activated = 0;
				bms->measures[measure_num]->channels[channel_num]->objects[i]->ypos = 0.0;
				bms->measures[measure_num]->channels[channel_num]->objects[i]->lane = bms->lane_channels[channel_num];
			}
		}

		return 1;
	}

	return 0;
}

// Determine what kind of chart this is, so we know how to render it later
static void determine_format(BMS* bms) {
	// BME
	if (strcmp(bms->extension, "bme") == 0) {
		bms->format = FORMAT_BME;
	}
	// BMS
	else if (strcmp(bms->extension, "bms") == 0) {
		bms->format = FORMAT_BMS;
	}
	// PMS
	else if (strcmp(bms->extension, "pms") == 0) {
		bms->format = FORMAT_PMS;
	}
}

static void map_channel_to_lane(BMS* bms, const char* channel, int lane) {
	bms->lane_channels[strtol(channel, NULL, 36)] = lane;
}

// Initialize the channel-to-lane lookup table, depending on format
static void init_lane_channels(BMS* bms) {
	for (int i = 0; i < 1295; i++) {
		bms->lane_channels[i] = -1;
	}

	switch (bms->format) {
		case FORMAT_BME:
		case FORMAT_BMS: {
			// 1P lanes
			map_channel_to_lane(bms, "16", 0); // 1P scratch
			map_channel_to_lane(bms, "11", 1); // 1P key 1
			map_channel_to_lane(bms, "12", 2); // 1P key 2
			map_channel_to_lane(bms, "13", 3); // 1P key 3
			map_channel_to_lane(bms, "14", 4); // 1P key 4
			map_channel_to_lane(bms, "15", 5); // 1P key 5
			map_channel_to_lane(bms, "18", 6); // 1P key 6
			map_channel_to_lane(bms, "19", 7); // 1P key 7
			break;
		}

		case FORMAT_PMS: {
			// popn buttons 1-9
			map_channel_to_lane(bms, "11", 0);
			map_channel_to_lane(bms, "12", 1);
			map_channel_to_lane(bms, "13", 2);
			map_channel_to_lane(bms, "14", 3);
			map_channel_to_lane(bms, "15", 4);
			map_channel_to_lane(bms, "22", 5);
			map_channel_to_lane(bms, "23", 6);
			map_channel_to_lane(bms, "24", 7);
			map_channel_to_lane(bms, "25", 8);
		}

		default: break;
	}
}

// Calculate the Y position relative to beginning of the measure for each note
static void calculate_object_positions(BMS* bms) {
	for (int i = 0; i < bms->measure_count; i++) {
		if (bms->measures[i] == NULL || bms->measures[i]->channels == NULL) {
			continue;
		}

		for (int j = 0; j < bms->measures[i]->channel_count; j++) {
			if (bms->measures[i]->channels[j] == NULL || bms->measures[i]->channels[j]->objects == NULL) {
				continue;
			}

			for (int k = 0; k < bms->measures[i]->channels[j]->object_count; k++) {
				if (bms->measures[i]->channels[j]->objects[k] == NULL || bms->measures[i]->channels[j]->objects[k]->id == 0) {
					continue;
				}

				bms->measures[i]->channels[j]->objects[k]->ypos = 1.0 - (double)k / bms->measures[i]->channels[j]->object_count;
			}
		}
	}
}

static void calculate_total_measures(BMS* bms) {
	for (int i = 0; i < bms->measure_count; i++) {
		if (bms->measures[i] == NULL) {
			continue;
		}

		bms->total_measures++;
	}
}

// Parse a BMS chart from a file and load it into a structure
BMS* BMS_load(const char* path) {
	FILE* fp = fopen(path, "r");

	if (!fp) {
		return NULL;
	}

	BMS* bms = malloc(sizeof(BMS));

	// Copy the path in order to get basename and dirname
	char file[1024];
	strcpy(file, path);

	// Initialize metadata fields
	bms->file = strdup(basename(file));
	bms->extension = strdup(get_extension(file));
	bms->directory = strdup(dirname(file));
	bms->play_type = PLAY_SINGLE;
	bms->genre = DEFAULT_GENRE;
	bms->title = DEFAULT_TITLE;
	bms->subtitle = DEFAULT_SUBTITLE;
	bms->init_bpm = DEFAULT_BPM;
	bms->play_level = DEFAULT_PLAYLEVEL;
	bms->rank = DEFAULT_RANK;
	bms->total = DEFAULT_TOTAL;
	bms->artist = DEFAULT_ARTIST;
	bms->maker = DEFAULT_MAKER;
	bms->subartists = NULL;
	bms->subartist_count = 0;
	bms->comments = NULL;
	bms->comment_count = 0;

	// Initialize definition fields
	bms->wav_defs = NULL;
	bms->wav_def_count = 0;
	bms->bmp_defs = NULL;
	bms->bmp_def_count = 0;
	bms->text_defs = NULL;
	bms->text_def_count = 0;
	bms->bpm_defs = NULL;
	bms->bpm_def_count = 0;
	bms->measures = NULL;
	bms->measure_count = 0;

	// Determine the format of this chart
	determine_format(bms);

	// Initialize the channel-to-lane lookup table
	init_lane_channels(bms);

	// Iterate through each line and parse commands
	char line[4096] = "";
	while (fgets(line, sizeof line, fp)) {
		char* command = strstr(line, "#");

		// If the line does not start with #, it's considered a comment
		if (command == NULL) {
			continue;
		}

		trim(command);

		// Parse metadata
		if (parse_player(bms, command)) continue;
		if (parse_genre(bms, command)) continue;
		if (parse_artist(bms, command)) continue;
		if (parse_subartist(bms, command)) continue;
		if (parse_maker(bms, command)) continue;
		if (parse_title(bms, command)) continue;
		if (parse_subtitle(bms, command)) continue;
		if (parse_bpm(bms, command)) continue;
		if (parse_rank(bms, command)) continue;
		if (parse_total(bms, command)) continue;
		if (parse_volwav(bms, command)) continue;

		// Parse definitions
		if (parse_wav(bms, command)) continue;
		if (parse_bmp(bms, command)) continue;
		if (parse_text(bms, command)) continue;
		if (parse_comment(bms, command)) continue;
		if (parse_bpmex(bms, command)) continue;

		// Parse lines
		if (parse_line(bms, command)) continue;
	}

	// Initialize helpers
	bms->elapsed = 0;
	bms->current_actual_measure = 0.0;
	bms->current_measure_part = 0.0;
	bms->current_measure = 0;
	bms->total_measures = 0;
	bms->current_bpm = bms->init_bpm;
	bms->mps = 1 / measure_duration(bms->current_bpm, 1.0);

	// Calculate the visual positions for each note
	calculate_object_positions(bms);

	// Calculate the total actual number of measures
	calculate_total_measures(bms);

	return bms;
}

// Process one logical step of a BMS chart
void BMS_step(BMS* bms, long dt) {
	bms->elapsed += dt;

	double last_measure = bms->current_actual_measure;
	int last_measure_index = (int)last_measure;
	// double last_measure_part = last_measure - last_measure_index;

	bms->current_actual_measure += bms->mps * ((double)dt / 1000000000.0);
	int measure_index = (int)bms->current_actual_measure;
	bms->current_measure_part = bms->current_actual_measure - measure_index;

	while (bms->measures[measure_index] == NULL) {
		bms->current_actual_measure += 1.0;
		measure_index++;
		last_measure_index++;
	}

	Measure* measure = bms->measures[measure_index];

	// Process objects in this measure's BGM channel
	for (int i = 0; i < measure->bgm_channel_count; i++) {
		Channel* channel = measure->bgm_channels[i];

		if (measure->bgm_channels[i] == NULL) {
			continue;
		}

		int object_index = (int)(bms->current_measure_part * channel->object_count);

		Object* object = channel->objects[object_index];

		if (object == NULL) {
			printf("Object is null!\n");
			continue;
		}

		// Don't process rests
		if (object->id == 0) {
			continue;
		}

		if (!object->activated && bms->wav_defs[object->id] != NULL) {
			Mixer_add(bms->wav_defs[object->id]->data, bms->wav_defs[object->id]->size);
			object->activated = 1;
		}
	}

	// Process objects in all other channels in the current measure
	for (int i = 0; i < measure->channel_count; i++) {
		Channel* channel = measure->channels[i];

		if (measure->channels[i] == NULL) {
			continue;
		}

		int object_index = (int)(bms->current_measure_part * channel->object_count);

		Object* object = channel->objects[object_index];

		if (object == NULL) {
			printf("Object is null!\n");
			continue;
		}

		// Don't process rests
		if (object->id == 0) {
			continue;
		}

		// Mix audio channels
		if (is_wav_channel(i)) {
			if (!object->activated && bms->wav_defs[object->id] != NULL) {
				Mixer_add(bms->wav_defs[object->id]->data, bms->wav_defs[object->id]->size);
				object->activated = 1;
			}
		}
	}

	// If this is a new measure, recalculate some things
	if (measure_index > last_measure_index) {
		bms->current_measure++;
		bms->mps = 1 / measure_duration(bms->current_bpm, measure->metre);
	}
}

// Returns all renderable objects (notes) for the whole chart
Measure** BMS_get_renderable_objects(BMS* bms) {
	// Create a structure with the same number of measures
	Measure** measures = calloc(bms->total_measures, sizeof(Measure*));

	// Note: we keep a secondary counter m (measure index), because oftentimes charts
	// will begin on measure 1 with a nonexistent measure 0, and we want to skip
	// measures that don't exist like we do in the step logic.
	int m = 0;
	for (int i = 0; i < bms->measure_count; i++) {
		if (bms->measures[i] == NULL) {
			continue;
		}

		// Create a new measure in the output array
		measures[m] = calloc(1, sizeof(Measure));

		// Count the visible channels
		int visible_channels[1400];
		int visible_channel_count = 0;

		for (int j = 0; j < 1400; j++) {
			visible_channels[j] = -1;
		}

		for (int j = 0; j < bms->measures[i]->channel_count; j++) {
			if (bms->measures[i]->channels[j] == NULL) {
				continue;
			}

			if (is_visible_channel(j)) {
				visible_channels[visible_channel_count++] = j;
			}
		}

		// Record the number of visible channels
		measures[m]->channel_count = visible_channel_count;

		// Don't do anything for this measure if there are no visible channels
		if (visible_channel_count == 0) {
			m++;
			continue;
		}

		// Allocate channels to the measure for every visible channel
		measures[m]->channels = calloc(visible_channel_count, sizeof(Channel*));

		for (int j = 0; j < visible_channel_count; j++) {
			// Allocate memory for this visible channel
			measures[m]->channels[j] = calloc(1, sizeof(Channel));

			// The index of this visible channel in the master data structure
			int v = visible_channels[j];

			// Count the visible objects in this channel
			int visible_object_count = 0;
			int visible_objects[1400];

			for (int k = 0; k < 1400; k++) {
				visible_objects[k] = -1;
			}

			for (int k = 0; k < bms->measures[i]->channels[v]->object_count; k++) {
				if (bms->measures[i]->channels[v]->objects[k]->visible) {
					visible_objects[visible_object_count++] = k;
				}
			}

			measures[m]->channels[j]->object_count = visible_object_count;

			// Don't do anything for this channel if there are no visible objects
			if (visible_object_count == 0) {
				continue;
			}

			// Allocate enough memory for the returned array of visible objects
			measures[m]->channels[j]->objects = calloc(visible_object_count, sizeof(Object*));

			// Copy visible objects to returned channel
			for (int k = 0; k < visible_object_count; k++) {
				// Allocate memory for this visible object
				measures[m]->channels[j]->objects[k] = calloc(1, sizeof(Object));

				// Get index of this visible object in the master data structure
				int o = visible_objects[k];

				// Copy this visible object from the master data structure to the filtered one
				memcpy(
					measures[m]->channels[j]->objects[k],
					bms->measures[i]->channels[v]->objects[o],
					sizeof(Object)
				);
			}
		}

		m++;
	}

	return measures;
}

// Free all memory used by a BMS structure
// TODO: need to add BGM channels
void BMS_free(BMS* bms) {
	if (bms == NULL) {
		return;
	}

	// Free the file name
	free(bms->file);

	// Free the extension
	free(bms->extension);

	// Free the directory name
	free(bms->directory);

	// Free subartists
	if (bms->subartists != NULL) {
		for (int i = 0; i < bms->subartist_count; i++) {
			free(bms->subartists[i]);
		}

		free(bms->subartists);
	}

	// Free wav definitions
	if (bms->wav_defs != NULL) {
		for (int i = 0; i < bms->wav_def_count; i++) {
			if (bms->wav_defs[i] != NULL) {
				free(bms->wav_defs[i]);
			}
		}

		free(bms->wav_defs);
	}

	// Free bitmap definitions
	if (bms->bmp_defs != NULL) {
		for (int i = 0; i < bms->bmp_def_count; i++) {
			if (bms->bmp_defs[i] != NULL) {
				free(bms->bmp_defs[i]);
			}
		}

		free(bms->bmp_defs);
	}

	// Free text defs
	if (bms->text_defs) {
		for (int i = 0; i < bms->text_def_count; i++) {
			if (bms->text_defs[i] != NULL) {
				free(bms->text_defs[i]);
			}
		}

		free(bms->text_defs);
	}

	// Free BPM defs
	free(bms->bpm_defs);

	// Free measures
	if (bms->measures != NULL) {
		for (int i = 0; i < bms->measure_count; i++) {
			// Free channels
			if (bms->measures[i] != NULL) {
				for (int j = 0; j < bms->measures[i]->channel_count; j++) {
					// Free objects
					if (bms->measures[i]->channels[j] != NULL) {
						for (int k = 0; k < bms->measures[i]->channels[j]->object_count; k++) {
							if (bms->measures[i]->channels[j]->objects[k] != NULL) {
								free(bms->measures[i]->channels[j]->objects[k]);
							}
						}
						free(bms->measures[i]->channels[j]);
					}
				}
				free(bms->measures[i]);
			}
		}
		free(bms->measures);
	}

	// Free the base struct
	free(bms);
}

// Print out BMS header data to the console
void BMS_print_info(BMS* bms) {
	printf("Play type: %d\n", bms->play_type);
	printf("Genre    : %s\n", bms->genre);
	printf("Title    : %s\n", bms->title);
	printf("Init BPM : %f\n", bms->init_bpm);
	printf("Rank     : %d\n", bms->rank);
	printf("Artist   : %s\n", bms->artist);
	printf("Maker    : %s\n", bms->maker);

	// Print subartists
	for (int i = 0; i < bms->subartist_count; i++) {
		if (bms->subartists[i] != NULL) {
			printf("Subartist %d = %s\n", i, bms->subartists[i]);
		}
	}

	// Print comments
	for (int i = 0; i < bms->comment_count; i++) {
		if (bms->comments[i] != NULL) {
			printf("Comment %d = %s\n", i, bms->comments[i]);
		}
	}

	// Wav defs
	for (int i = 0; i < bms->wav_def_count; i++) {
		if (bms->wav_defs[i] != NULL) {
			printf("Wav %d = %s\n", i, bms->wav_defs[i]->file);
		}
	}

	// BMP defs
	for (int i = 0; i < bms->bmp_def_count; i++) {
		if (bms->bmp_defs[i] != NULL) {
			printf("Bmp %d = %s\n", i, bms->bmp_defs[i]->file);
		}
	}

	// Print text defs
	for (int i = 0; i < bms->text_def_count; i++) {
		if (bms->text_defs[i] != NULL) {
			printf("Text %d = %s\n", i, bms->text_defs[i]);
		}
	}

	// Print BPM defs
	for (int i = 0; i < bms->bpm_def_count; i++) {
		if (bms->bpm_defs[i] != 0) {
			printf("Bpm %d = %f\n", i, bms->bpm_defs[i]);
		}
	}

	// Print measures
	if (bms->measures != NULL) {
		printf("%d measures\n", bms->measure_count);
		for (int i = 0; i < bms->measure_count; i++) {
			if (bms->measures[i] != NULL) {
				printf("\nMeasure %d:\n", i);
				for (int j = 0; j < bms->measures[i]->channel_count; j++) {
					if (bms->measures[i]->channels[j] != NULL) {
						printf("Channel %d: ", j);
						for (int k = 0; k < bms->measures[i]->channels[j]->object_count; k++) {
							if (bms->measures[i]->channels[j]->objects[k] != NULL) {
								printf("%d ", bms->measures[i]->channels[j]->objects[k]->id);
							}
						}
						printf("\n");
					}
				}
			}
		}
	}
}
