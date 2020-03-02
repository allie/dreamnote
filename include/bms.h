#ifndef BMS_H
#define BMS_H

#include <stdlib.h>

// Formats
enum {
	FORMAT_BMS = 0,
	FORMAT_BME,
	FORMAT_BME_FP,
	FORMAT_PMS,
	FORMAT_PMS_BME
};

// Play types
enum {
	PLAY_SINGLE = 1,
	PLAY_COUPLE = 2,
	PLAY_DOUBLE = 3,
	PLAY_BATTLE = 4
};

// Common channels
#define CHANNEL_BGM          1
#define CHANNEL_METRE        2
#define CHANNEL_BPM_CHANGE   3
#define CHANNEL_BGA          4
#define CHANNEL_POOR_CHANGE  6
#define CHANNEL_LAYER        7
#define CHANNEL_EXTENDED_BPM 8
#define CHANNEL_STOP         9

// BMS + BME channels (IIDX-like)
// 1P turntable
#define CHANNEL_BMS_1P_SCRATCH 42 // 16
// 1P keys
#define CHANNEL_BMS_1P_KEY1    37 // 11
#define CHANNEL_BMS_1P_KEY2    38 // 12
#define CHANNEL_BMS_1P_KEY3    39 // 13
#define CHANNEL_BMS_1P_KEY4    40 // 14
#define CHANNEL_BMS_1P_KEY5    41 // 15
#define CHANNEL_BMS_1P_KEY6    44 // 18
#define CHANNEL_BMS_1P_KEY7    45 // 19
// 2P keys
#define CHANNEL_BMS_2P_KEY1    73 // 21
#define CHANNEL_BMS_2P_KEY2    74 // 22
#define CHANNEL_BMS_2P_KEY3    75 // 23
#define CHANNEL_BMS_2P_KEY4    76 // 24
#define CHANNEL_BMS_2P_KEY5    77 // 25
#define CHANNEL_BMS_2P_KEY6    80 // 28
#define CHANNEL_BMS_2P_KEY7    81 // 29
// 2P turntable
#define CHANNEL_BMS_2P_SCRATCH 78 // 26

// PMS channels (pop'n-like)
#define CHANNEL_PMS_BUTTON1 37
#define CHANNEL_PMS_BUTTON2 38
#define CHANNEL_PMS_BUTTON3 39
#define CHANNEL_PMS_BUTTON4 40
#define CHANNEL_PMS_BUTTON5 41
#define CHANNEL_PMS_BUTTON6 74
#define CHANNEL_PMS_BUTTON7 75
#define CHANNEL_PMS_BUTTON8 76
#define CHANNEL_PMS_BUTTON9 77

// Defaults
#define DEFAULT_PLAYTYPE PLAY_SINGLE
#define DEFAULT_GENRE NULL
#define DEFAULT_TITLE NULL
#define DEFAULT_SUBTITLE NULL
#define DEFAULT_ARTIST NULL
#define DEFAULT_MAKER NULL
#define DEFAULT_BPM 130
#define DEFAULT_PLAYLEVEL 0
#define DEFAULT_RANK 0
#define DEFAULT_TOTAL 160.0

// A wav object definition
// #WAVxx <filename>
typedef struct {
	char* file;
	float* data;
	size_t size;
} WavDef;

// A bitmap object definition
// #BMPxx <filename>
typedef struct {
	char* file;
} BmpDef;

// A channel object
typedef struct {
	int id;
	int visible;
	int activated;
	double ypos;
	int lane;
	double timing;
	int judgment;
} Object;

// An internal representation of a single channel/column.
typedef struct {
	Object** objects;
	int object_count;
} Channel;

// An internal representation of one measure
typedef struct {
	Channel** channels;
	Channel** bgm_channels;
	int channel_count;
	int bgm_channel_count;
	double metre;
} Measure;

// A parsed BMS chart
typedef struct {
	// Info fields
	char* file;
	char* extension;
	char* directory;
	int play_type;
	char* genre;
	char* title;
	char* subtitle;
	char* artist;
	char* maker;
	char** subartists;
	int subartist_count;
	char** comments;
	int comment_count;
	double init_bpm;
	int play_level;
	int rank;
	double total;
	double volwav;

	// Definition arrays
	WavDef** wav_defs;
	int wav_def_count;
	BmpDef** bmp_defs;
	int bmp_def_count;
	char** text_defs;
	int text_def_count;
	double* bpm_defs;
	int bpm_def_count;
	Measure** measures;
	int measure_count;

	// Helper fields
	long elapsed;
	double current_actual_measure;
	double current_measure_part;
	int current_measure;
	int total_measures;
	double current_bpm;
	double mps;
	long measure_duration;
	int format;
	int lane_channels[1295];
} BMS;

BMS* BMS_load(const char* path);
void BMS_step(BMS* bms, long dt);
void BMS_handle_button_press(BMS* bms, int lane);
Measure** BMS_get_renderable_objects(BMS* bms);
void BMS_free(BMS* bms);
void BMS_print_info(BMS* bms);

#endif
