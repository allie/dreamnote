#include "mixer.h"
#include "log.h"

#include <stdio.h>
#include <portaudio.h>
#include <sndfile.h>
#include <samplerate.h>

#define NUM_CHANNELS 2048
#define SAMPLE_RATE 44100

typedef struct {
	float* data;
	size_t size;
	int index;
	int finished;
} Channel;

static PaStream* stream = NULL;
static Channel channels[NUM_CHANNELS];
static int sample_rate;
static int buffer_size;
// static int state = MIXER_PLAYING;
static float volume = 0.5f;

// Mix the samples of every channel into one
static float mix_samples() {
	float sample = 0.0f;

	// Sum up all channels
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (channels[i].finished) {
			continue;
		}

		sample += channels[i].data[channels[i].index];

		channels[i].index++;

		if (channels[i].index == channels[i].size) {
			channels[i].finished = 1;
		}
	}

	// Adjust to the master volume
	sample *= volume;

	// Clip the sample
	if (sample > 1.0f) {
		sample = 1.0f;
	} else if (sample < -1.0f) {
		sample = -1.0f;
	}

	return sample;
}

// PortAudio callback
static int Mixer_PACallback(const void* input, void* output, unsigned long frame_count,
	const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* user_data) {
	float* out = (float*)output;

	// For each frame, play back two mixed samples (one per stereo channel)
	for (int i = 0; i < frame_count; i++) {
		*out++ = mix_samples(); // Left
		*out++ = mix_samples(); // Right
	}

	return 0;
}

int Mixer_load_file(const char* path, float** buffer, size_t* size) {
	// Open the file
	SF_INFO info = {};
	SNDFILE* file = sf_open(path, SFM_READ, &info);

	if (file == NULL) {
		Log_error("Error opening sound file: %s\n", sf_strerror(file));
		return 0;
	}

	// Prepare libsamplerate
	SRC_DATA data = {};
	data.src_ratio = SAMPLE_RATE / (double)info.samplerate;
	data.input_frames = info.frames;
	data.output_frames = (int)(info.frames * data.src_ratio) + 1;
	float* input_buffer = malloc(sizeof(float) * info.frames * info.channels);
	data.data_out = malloc(sizeof(float) * data.output_frames * info.channels);

	// Read the audio data from the file
	sf_count_t items_read = sf_read_float(file, input_buffer, info.frames * info.channels);

	if (items_read != info.frames * info.channels) {
		Log_error("Read %lld samples instead of %lld\n!", items_read, info.frames * info.channels);
		return 0;
	}

	sf_close(file);

	data.data_in = input_buffer;

	// Convert the sample rate to 44.1khz
	int error = src_simple(&data, SRC_SINC_FASTEST, info.channels);

	if (error) {
		Log_error("Error converting sample rate: %s\n", src_strerror(error));
		return 0;
	}

	free(input_buffer);

	*size = data.output_frames_gen * 2;

	// Convert mono to stereo if necessary
	if (info.channels == 1) {
		float* stereo = malloc(sizeof(float) * *size);

		for (int i = 0; i < data.output_frames_gen; i++) {
			stereo[i * 2] = data.data_out[i];
			stereo[i * 2 + 1] = data.data_out[i];
		}

		free(data.data_out);
		*buffer = stereo;
	}
	// Do nothing for stereo
	else if (info.channels == 2) {
		*buffer = data.data_out;
	}
	// 0 channels or >2 is not supported right now
	else {
		Log_error("Unsupported number of channels.\n");
		 return 0;
	}

	// Log_debug("Chunk loaded and converted: %s, %dhz, %d channels\n", path, info.samplerate, info.channels);
	return 1;
}

// Initialize the mixer
int Mixer_init(int rate, int buffer) {
	// Set the sample rate
	sample_rate = rate;

	// Create the output buffer
	buffer_size = buffer;

	// Set all channels to null
	for (int i = 0; i < NUM_CHANNELS; i++) {
		channels[i].data = NULL;
		channels[i].size = 0;
		channels[i].index = 0;
		channels[i].finished = 1;
	}

	// Initialize PortAudio
	PaError error = Pa_Initialize();
	if (error != paNoError) {
		Log_fatal("PortAudio error: %s\n", Pa_GetErrorText(error));
		return 0;
	}

	Log_debug("Successfully initialized PortAudio");

	// Open the output stream
	error = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, sample_rate, buffer_size, Mixer_PACallback, NULL);
	if (error != paNoError) {
		Log_fatal("PortAudio error: %s\n", Pa_GetErrorText(error));
		return 0;
	}

	Log_debug("Successfully opened PortAudio output stream");

	// Start the stream
	error = Pa_StartStream(stream);
	if (error != paNoError) {
		Log_fatal("PortAudio error: %s\n", Pa_GetErrorText(error));
		return 0;
	}

	Log_debug("Successfully started PortAudio output stream");

	Log_debug("Successfully initialized Mixer");

	return 1;
}

// Adds a sample to the mix, using any free channel available.
// If there are no free channels available, -1 will be returned.
// Otherwise, the channel number that the audio is playing on will be returned.
int Mixer_add(float* data, size_t size) {
	int channel = -1;

	// Find the first channel that is marked as finished (ready to accept data)
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (channels[i].finished) {
			channel = i;
		}
	}

	// If there are no free channels, give up
	if (channel == -1) {
		return -1;
	}

	// Add the new sample
	channels[channel].data = data;
	channels[channel].size = size;
	channels[channel].index = 0;
	channels[channel].finished = 0;

	return channel;
}
