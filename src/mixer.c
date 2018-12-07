#include "mixer.h"

#include <stdio.h>
#include <sndfile.h>
#include <samplerate.h>
#include <soundio/soundio.h>

#define NUM_CHANNELS 2048
#define SAMPLE_RATE 44100

struct SoundIo* soundio = NULL;
struct SoundIoDevice* device = NULL;
struct SoundIoOutStream* outstream = NULL;

typedef struct {
	float* data;
	size_t size;
	int index;
	int finished;
} Channel;

static Channel* channels[NUM_CHANNELS];
static int sample_rate;
static int buffer_size;
// static int state = MIXER_PLAYING;
static float volume = 0.5f;

// Mix the samples of every channel into one
static float mix_samples() {
	float sample = 0.0f;

	// Sum up all channels
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (channels[i] == NULL || channels[i]->finished) {
			continue;
		}

		sample += channels[i]->data[channels[i]->index];

		channels[i]->index++;

		if (channels[i]->index == channels[i]->size) {
			channels[i]->finished = 1;
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

// SoundIo callback
static void write_callback(struct SoundIoOutStream* outstream, int frame_count_min, int frame_count_max) {
	const struct SoundIoChannelLayout* layout = &outstream->layout;
	struct SoundIoChannelArea* areas = NULL;
	int frames_left = frame_count_max;
	int error = 0;

	while (frames_left > 0) {
		int frame_count = frames_left;

		error = soundio_outstream_begin_write(outstream, &areas, &frame_count);
		if (error) {
            printf("SoundIo error: %s\n", soundio_strerror(error));
            exit(1);
        }

        if (frame_count == 0) {
        	break;
        }

        for (int i = 0; i < frame_count; i++) {
			for (int j = 0; j < layout->channel_count; j++) {
				float *ptr = (float*)(areas[j].ptr + areas[j].step * i);
				*ptr = mix_samples();
			}
        }

        frames_left -= frame_count;
	}
}

int Mixer_load_file(const char* path, float** buffer, size_t* size) {
	// Open the file
	SF_INFO info = {};
	SNDFILE* file = sf_open(path, SFM_READ, &info);

	if (file == NULL) {
		printf("Error opening sound file: %s\n", sf_strerror(file));
		return 0;
	}

	// Prepare libsamplerate
	SRC_DATA data = {};
	data.src_ratio = SAMPLE_RATE / (double)info.samplerate;
	data.input_frames = info.frames;
	data.output_frames = (int)(info.frames * data.src_ratio) + 1;
	float* input_buffer = calloc(info.frames * info.channels, sizeof(float));
	data.data_out = calloc(data.output_frames * info.channels, sizeof(float));

	// Read the audio data from the file
	sf_count_t items_read = sf_read_float(file, input_buffer, info.frames * info.channels);

	if (items_read != info.frames * info.channels) {
		printf("Read %lld samples instead of %lld\n!", items_read, info.frames * info.channels);
		return 0;
	}

	sf_close(file);

	data.data_in = input_buffer;

	// Convert the sample rate to 44.1khz
	int error = src_simple(&data, SRC_SINC_FASTEST, info.channels);

	if (error) {
		printf("Error converting sample rate: %s\n", src_strerror(error));
		return 0;
	}

	free(input_buffer);

	*size = data.output_frames_gen * 2;

	// Convert mono to stereo if necessary
	if (info.channels == 1) {
		float* stereo = calloc(*size, sizeof(float));

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
		printf("Unsupported number of channels.\n");
		 return 0;
	}

	// printf("Chunk loaded and converted: %s, %dhz, %d channels\n", path, info.samplerate, info.channels);
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
		if (channels[i] != NULL) {
			free(channels[i]);
		}

		channels[i] = NULL;
	}

	// Set up SoundIo
	soundio = soundio_create();
	if (!soundio) {
		printf("Could not initialize SoundIo.\n");
		return 0;
    }

    int error = soundio_connect(soundio);
	if (error) {
    	printf("Could not connect to SoundIo: %s\n", soundio_strerror(error));
    	return 0;
	}

    soundio_flush_events(soundio);

    device = soundio_get_output_device(soundio, soundio_default_output_device_index(soundio));
	if (!device) {
		printf("Could not get SoundIo device.\n");
		return 0;
	}

	outstream = soundio_outstream_create(device);
	if (!outstream) {
		printf("Could not get SoundIo out stream.\n");
		return 0;
	}
	outstream->format = SoundIoFormatFloat32NE;
	outstream->sample_rate = sample_rate;
	outstream->software_latency = buffer_size;
	outstream->write_callback = write_callback;

	error = soundio_outstream_open(outstream);
	if (error) {
		printf("Could not open SoundIo out stream: %s", soundio_strerror(error));
		return 1;
	}

	if (outstream->layout_error) {
		printf("SoundIo layout error: %s\n", soundio_strerror(outstream->layout_error));
	}

	error = soundio_outstream_start(outstream);
	if (error) {
		printf("Could not start SoundIo out stream: %s\n", soundio_strerror(error));
		return 1;
	}

	return 1;
}

// Adds a sample to the mix, using any free channel available.
// If there are no free channels available, -1 will be returned.
// Otherwise, the channel number that the audio is playing on will be returned.
int Mixer_add(float* data, size_t size) {
	int channel = -1;

	// Clean up finished channels
	// TODO: this should be rethought so we don't have to wait for a new channel
	// to be added in order to free channels. Freeing from within the callback isn't
	// feasible, so perhaps try kicking off another thread to free channels?
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (channels[i] != NULL && channels[i]->finished) {
			free(channels[i]);
			channels[i] = NULL;
		}
	}

	// Look for a free channel
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (channels[i] == NULL) {
			channel = i;
			break;
		}
	}

	// If there are no free channels, give up
	if (channel == -1) {
		return -1;
	}

	// Initialize the channel
	channels[channel] = calloc(1, sizeof(Channel));
	channels[channel]->data = data;
	channels[channel]->size = size;
	channels[channel]->index = 0;
	channels[channel]->finished = 0;

	return channel;
}
