#include <stdlib.h>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>
#include <rfxgen.h>

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: rfxplay file.sfx out.wav\n");
		return EXIT_FAILURE;
	}

	WaveParams *wp = LoadWaveParams(argv[1]);
	Wave raw = GenerateWave(wp);

	/* Write WAV file. */
	{
		drwav_data_format format;
		drwav wav;

		format.container = drwav_container_riff;
		format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
		format.channels = raw.channels;
		format.sampleRate = WAVE_SAMPLE_RATE;
		format.bitsPerSample = raw.sampleSize;

		if(drwav_init_file_write(&wav, argv[2], &format, NULL) != DRWAV_TRUE)
		{
			fprintf(stderr, "Error writing wav file.\n");
			return EXIT_FAILURE;
		}

		drwav_write_pcm_frames(&wav, raw.sampleCount, raw.data);
		drwav_uninit(&wav);
	}

	return EXIT_SUCCESS;
}
