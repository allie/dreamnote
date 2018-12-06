#ifndef BMS_H
#define BMS_H

#include <stdlib.h>

// Play types
#define PLAY_SINGLE 1
#define PLAY_COUPLE 2
#define PLAY_DOUBLE 3
#define PLAY_BATTLE 4

// Common channels
#define CHANNEL_BGM 0x01
#define CHANNEL_METRE 0x02
#define CHANNEL_BPM_CHANGE 0x03
#define CHANNEL_BGA 0x04
#define CHANNEL_POOR_CHANGE 0x06
#define CHANNEL_LAYER 0x07
#define CHANNEL_EXTENDED_BPM 0x08
#define CHANNEL_STOP 0x09

// BMS channels (IIDX-like)
#define CHANNEL_IIDX_KEY1 0x25
#define CHANNEL_IIDX_KEY2 0x26
#define CHANNEL_IIDX_KEY3 0x27
#define CHANNEL_IIDX_KEY4 0x28
#define CHANNEL_IIDX_KEY5 0x29
#define CHANNEL_IIDX_KEY6 0x2C
#define CHANNEL_IIDX_KEY7 0x2D
#define CHANNEL_IIDX_SCRATCH 0x2A

// PMS channels (pop'n-like)
#define CHANNEL_POPN_BUTTON1 0x11
#define CHANNEL_POPN_BUTTON2 0x12
#define CHANNEL_POPN_BUTTON3 0x13
#define CHANNEL_POPN_BUTTON4 0x14
#define CHANNEL_POPN_BUTTON5 0x15
#define CHANNEL_POPN_BUTTON6 0x22
#define CHANNEL_POPN_BUTTON7 0x23
#define CHANNEL_POPN_BUTTON8 0x24
#define CHANNEL_POPN_BUTTON9 0x25

// O2Mania channels
// Note: this is for O2Mania 1.2. Newer versions will use
// the IIDX-style keys, determined by the presence of objects
// in channel 19.
#define CHANNEL_O2_P1_KEY1 0x16
#define CHANNEL_O2_P1_KEY2 0x11
#define CHANNEL_O2_P1_KEY3 0x12
#define CHANNEL_O2_P1_KEY4 0x13
#define CHANNEL_O2_P1_KEY5 0x14
#define CHANNEL_O2_P1_KEY6 0x15
#define CHANNEL_O2_P1_KEY7 0x18

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
	int activated;
	double ypos;
	int lane;
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
	double elapsed;
	double current_actual_measure;
	double current_measure_part;
	int current_measure;
	int total_measures;
	double current_bpm;
	double mps;
} BMS;

BMS* BMS_load(const char* path);
void BMS_step(BMS* bms, double dt);
Measure** BMS_get_renderable_objects(BMS* bms);
void BMS_free(BMS* bms);
void BMS_print_info(BMS* bms);

#endif
