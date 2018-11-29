#ifndef BMS_H
#define BMS_H

// Play types
#define PLAY_SINGLE 1
#define PLAY_COUPLE 2
#define PLAY_DOUBLE 3
#define PLAY_BATTLE 4

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

// BMS channels (IIDX-like)
#define CHANNEL_BGM 1
#define CHANNEL_TIME_SIGNATURE 2
#define CHANNEL_BPM_CHANGE 3
#define CHANNEL_BGA 4
#define CHANNEL_POOR_CHANGE 6
#define CHANNEL_LAYER 7
#define CHANNEL_EXTENDED_BPM 8
#define CHANNEL_STOP 9

// PMS channels (pop'n-like)
#define CHANNEL_POPN_BUTTON1 11
#define CHANNEL_POPN_BUTTON2 12
#define CHANNEL_POPN_BUTTON3 13
#define CHANNEL_POPN_BUTTON4 14
#define CHANNEL_POPN_BUTTON5 15
#define CHANNEL_POPN_BUTTON6 22
#define CHANNEL_POPN_BUTTON7 23
#define CHANNEL_POPN_BUTTON8 24
#define CHANNEL_POPN_BUTTON9 25

// O2Mania channels
// Note: this is for O2Mania 1.2. Newer versions will use
// the IIDX-style keys, determined by the presence of objects
// in channel 19.
#define CHANNEL_O2_P1_KEY1 16
#define CHANNEL_O2_P1_KEY2 11
#define CHANNEL_O2_P1_KEY3 12
#define CHANNEL_O2_P1_KEY4 13
#define CHANNEL_O2_P1_KEY5 14
#define CHANNEL_O2_P1_KEY6 15
#define CHANNEL_O2_P1_KEY7 18

// A wav object definition
// #WAVxx <filename>
typedef struct {
	char* file;
} WavDef;

// A bitmap object definition
// #BMPxx <filename>
typedef struct {
	char* file;
} BmpDef;

// A channel object
typedef struct {
	int id;
} Object;

// An internal representation of a single channel/column.
typedef struct {
	Object** objects;
	int object_count;
} Channel;

// An internal representation of one measure
typedef struct {
	Channel** channels;
	int channel_count;
	double timestamp;
} Measure;

// A parsed BMS chart
typedef struct {
	// Info fields
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
} BMS;

BMS* BMS_load(const char* path);
void BMS_step(BMS* bms, double dt);
void BMS_free(BMS* bms);
void BMS_print_info(BMS* bms);

#endif
