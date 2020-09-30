#include <stdlib.h>
#include <stdio.h>

#include <rfxgen.h>

int main(int argc, char *argv[])
{
	FILE *out;

	if(argc != 3)
	{
		fprintf(stderr, "Usage: rfxplay file.sfx out.wav\n");
		return EXIT_FAILURE;
	}

	WaveParams *wp = LoadWaveParams(argv[1]);
	Wave wav = GenerateWave(wp);

	out = fopen(argv[2], "wb");
	fwrite(wav.data, wav.sampleSize/8, wav.sampleCount * wav.channels, out);
	fclose(out);

	return EXIT_SUCCESS;
}
