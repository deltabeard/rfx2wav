
#pragma once

#define MAX_WAVE_LENGTH_SECONDS  10     // Max length for wave: 10 seconds
#define WAVE_SAMPLE_RATE      44100     // Default sample rate

typedef struct WaveParams WaveParams;

// Wave type, defines audio wave data
typedef struct Wave {
	unsigned int sampleCount;       // Total number of samples
	unsigned int sampleRate;        // Frequency (samples per second)
	unsigned int sampleSize;        // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
	unsigned int channels;          // Number of channels	(1-mono, 2-stereo)
	void *data;                     // Buffer data pointer
} Wave;

WaveParams *LoadWaveParams(const char *fileName);                 // Load wave parameters from file
Wave GenerateWave(WaveParams *params);                            // Generate wave data from parameters
