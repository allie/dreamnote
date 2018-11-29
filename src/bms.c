#include "bms.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Parse a BMS chart from a file and load it into a structure
BMS* BMS_load(const char* path) {
	FILE* fp = fopen(path, "r");

	if (!fp) {
		return NULL;
	}

	BMS* bms = malloc(sizeof(BMS));

	// Initialize default values
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

	// Initialize definitions
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

	// Initialize helpers
	bms->elapsed = 0.0f;

	// Iterate through each line and parse
	char line[4096] = "";
	while (fgets(line, sizeof line, fp)) {
		char* command = strstr(line, "#");

		// If the line does not start with #, it's considered a comment
		if (command == NULL) {
			continue;
		}

		// #PLAYER x
		if (stristr(command, "#PLAYER")) {
			command += strlen("#PLAYER");
			trim(command);
			long value = strtol(command, NULL, 10);

			if (value >= PLAY_SINGLE && value <= PLAY_BATTLE) {
				bms->play_type = (int)value;
			}
		}

		// #GENRE x
		// Due to a typo at some point in BMS history, GENLE is also accepted
		else if (stristr(command, "#GENRE") || stristr(command, "#GENLE")) {
			command += strlen("#GENRE");
			trim(command);
			bms->genre = strdup(command);
		}

		// #ARTIST x
		else if (stristr(command, "#ARTIST")) {
			command += strlen("#ARTIST");
			trim(command);
			bms->artist = strdup(command);
		}

		// #SUBARTIST x
		else if (stristr(command, "#SUBARTIST")) {
			command += strlen("#SUBARTIST");

			// Resize the defs array
			int old_count = bms->subartist_count;
			bms->subartist_count++;
			bms->subartists = recalloc(bms->subartists, sizeof(char*), old_count, bms->subartist_count);

			trim(command);

			// Create a new entry in the defs array
			bms->subartists[old_count] = strdup(command);
		}

		// #MAKER x
		else if (stristr(command, "#MAKER")) {
			command += strlen("#MAKER");
			trim(command);
			bms->maker = strdup(command);
		}

		// #TITLE x
		else if (stristr(command, "#TITLE")) {
			command += strlen("#TITLE");
			trim(command);
			bms->title = strdup(command);
		}

		// #SUBTITLE x
		else if (stristr(command, "#SUBTITLE")) {
			command += strlen("#SUBTITLE");
			trim(command);
			bms->subtitle = strdup(command);
		}

		// #BPM x
		// Not to be confused with #BPMxx yy
		else if (stristr(command, "#BPM") && iswhitespace(command[4])) {
			command += strlen("#BPM");
			trim(command);
			bms->init_bpm = strtod(command, NULL);
		}

		// #RANK x
		else if (stristr(command, "#RANK")) {
			command += strlen("#RANK");
			trim(command);
			long value = strtol(command, NULL, 10);
			bms->rank = (int)value;
		}

		// #TOTAL x
		else if (stristr(command, "#TOTAL")) {
			command += strlen("#TOTAL");
			trim(command);
			bms->total = strtod(command, NULL);
		}

		// #VOLWAV x
		else if (stristr(command, "#VOLWAV")) {
			command += strlen("#VOLWAV");
			trim(command);
			bms->volwav = strtod(command, NULL);
		}

		// #WAVxx <filename>
		// In modern BMS, xx is parsed in base 36 to allow more channels
		else if (stristr(command, "#WAV")) {
			command += strlen("#WAV");

			// Extract the ID (xx)
			char id_base36[] = {command[0], command[1]};
			long id = strtol(id_base36, NULL, 36);

			// Resize the defs array
			int old_count = bms->wav_def_count;
			bms->wav_def_count = id + 1;
			bms->wav_defs = recalloc(bms->wav_defs, sizeof(WavDef*), old_count, bms->wav_def_count);

			command += 2;
			trim(command);

			// Create a new entry in the defs array
			bms->wav_defs[id] = malloc(sizeof(WavDef));
			bms->wav_defs[id]->file = strdup(command);
		}

		// #BMPxx <filename>
		// In modern BMS, xx is parsed in base 36 to allow more channels
		else if (stristr(command, "#BMP")) {
			command += strlen("#BMP");

			// Extract the ID (xx)
			char id_base36[] = {command[0], command[1]};
			long id = strtol(id_base36, NULL, 36);

			// Resize the defs array
			int old_count = bms->bmp_def_count;
			bms->bmp_def_count = id + 1;
			bms->bmp_defs = recalloc(bms->bmp_defs, sizeof(BmpDef*), old_count, bms->bmp_def_count);

			command += 2;
			trim(command);

			// Create a new entry in the defs array
			bms->bmp_defs[id] = malloc(sizeof(BmpDef));
			bms->bmp_defs[id]->file = strdup(command);
		}

		// #TEXTxx "<message>"
		// #TEXTxx <message>
		// #SONGxx "<message>"
		// #SONGxx <message>
		// In modern BMS, xx is parsed in base 36 to allow more channels
		else if (stristr(command, "#TEXT")) {
			command += strlen("#TEXT");

			// Extract the ID (xx)
			char id_base36[] = {command[0], command[1]};
			long id = strtol(id_base36, NULL, 36);

			// Resize the defs array
			int old_count = bms->text_def_count;
			bms->text_def_count = id + 1;
			bms->text_defs = recalloc(bms->text_defs, sizeof(char*), old_count, bms->text_def_count);

			command += 2;
			trim(command);

			// Strip quotes
			if (command[0] == '"') {
				command++;
				command[strlen(command) - 1] = '\0';
			}

			// Create a new entry in the defs array
			bms->text_defs[id] = strdup(command);
		}

		// #COMMENT "<message>"
		// #COMMENT <message>
		else if (stristr(command, "#COMMENT")) {
			command += strlen("#COMMENT");

			// Resize the defs array
			int old_count = bms->comment_count;
			bms->comment_count++;
			bms->comments = recalloc(bms->comments, sizeof(char*), old_count, bms->comment_count);

			trim(command);

			// Strip quotes
			if (command[0] == '"') {
				command++;
				command[strlen(command) - 1] = '\0';
			}

			// Create a new entry in the defs array
			bms->comments[old_count] = strdup(command);
		}

		// #BPMxx <new BPM>
		// In modern BMS, xx is parsed in base 36 to allow more channels
		else if (stristr(command, "#BPM")) {
			command += strlen("#BPM");

			// Extract the ID (xx)
			char id_base36[] = {command[0], command[1]};
			long id = strtol(id_base36, NULL, 36);

			// Resize the defs array
			int old_count = bms->bpm_def_count;
			bms->bpm_def_count = id + 1;
			bms->bpm_defs = recalloc(bms->bpm_defs, sizeof(double*), old_count, bms->bpm_def_count);

			command += 2;
			trim(command);

			// Create a new entry in the defs array
			bms->bpm_defs[id] = strtod(command, NULL);
		}

		// #xxxyy:zz
		else if (command[6] == ':') {
			// Extract the measure number (xxx)
			char measure_str[] = {command[1], command[2], command[3]};
			long measure_num = strtol(measure_str, NULL, 10);

			// Extract the channel number
			char channel_str[] = {command[4], command[5]};
			long channel_num = strtol(channel_str, NULL, 10);

			// Extract the message
			char* message = command + strlen("#xxxyy:");

			// Resize the measures array if need be
			int old_count = bms->measure_count;
			bms->measure_count = measure_num + 1;
			bms->measures = recalloc(bms->measures, sizeof(Measure*), old_count, bms->measure_count);
			// bms->measures = realloc_zero(bms->measures, sizeof(Measure*) * old_count, sizeof(Measure*) * bms->measure_count);

			Measure* measure = bms->measures[measure_num];

			// If the measure doesn't exist, create it
			if (bms->measures[measure_num] == NULL) {
				bms->measures[measure_num] = malloc(sizeof(Measure));
				bms->measures[measure_num]->channel_count = 0;
				bms->measures[measure_num]->channels = NULL;
			}

			// Resize the channels array if need be
			old_count = bms->measures[measure_num]->channel_count;
			bms->measures[measure_num]->channel_count = channel_num + 1;
			bms->measures[measure_num]->channels = recalloc(
				bms->measures[measure_num]->channels,
				sizeof(Channel*),
				old_count,
				bms->measures[measure_num]->channel_count
			);

			Channel* channel = bms->measures[measure_num]->channels[channel_num];

			// If the channel doesn't exist, create it
			if (bms->measures[measure_num]->channels[channel_num] == NULL) {
				bms->measures[measure_num]->channels[channel_num] = malloc(sizeof(Channel));
				bms->measures[measure_num]->channels[channel_num]->objects = NULL;
			}

			// If an objects array already exists, free it so we can overwrite it
			else if (bms->measures[measure_num]->channels[channel_num]->objects != NULL) {
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
				char id_base36[] = {message[i * 2], message[i * 2 + 1]};
				long id = strtol(id_base36, NULL, 36);

				bms->measures[measure_num]->channels[channel_num]->objects[i] = malloc(sizeof(Object));
				bms->measures[measure_num]->channels[channel_num]->objects[i]->id = (int)id;
			}
		}
	}

	return bms;
}

// Process one logical step of a BMS chart
void BMS_step(BMS* bms, double dt) {
	bms->elapsed += dt;

	// Stuff
}

// Free all memory used by a BMS structure
void BMS_free(BMS* bms) {
	if (bms == NULL) {
		return;
	}

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
