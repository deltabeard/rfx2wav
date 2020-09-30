/*******************************************************************************************
*
*   rFXGen v2.2 - A simple and easy to use sounds generator (based on Tomas Petterson sfxr)
*
*   CONFIGURATION:
*
*   #define VERSION_ONE
*       Enable PRO features for the tool. Usually command-line and export options related.
*
*   #define COMMAND_LINE_ONLY
*       Compile tool only for command line usage
*
*   #define CUSTOM_MODAL_DIALOGS
*       Use custom raygui generated modal dialogs instead of native OS ones
*       NOTE: Avoids including tinyfiledialogs depencency library
*
*   #define RENDER_WAVE_TO_TEXTURE (defined by default)
*       Use RenderTexture2D to render wave on. If not defined, wave is diretly drawn using lines.
*
*   VERSIONS HISTORY:
*       2.2  (23-Feb-2019) Update to raylib 3.0, raygui 2.7 and adapted for web
*       2.1  (09-Sep-2019) Ported to latest raygui 2.6
*                          Support custom file dialogs (on non-DESKTOP platforms)
*                          Slight screen resize to adapt to new styles fonts
*       2.0  (xx-Nov-2018) GUI redesigned, CLI improvements
*       1.8  (10-Oct-2018) Functions renaming, code reorganized, better consistency...
*       1.5  (23-Sep-2018) Support .wav export to code and sound playing on command line
*       1.4  (15-Sep-2018) Redesigned command line and comments
*       1.3  (15-May-2018) Reimplemented gui using rGuiLayout
*       1.2  (16-Mar-2018) Working on some code improvements and GUI review
*       1.1  (01-Oct-2017) Code review, simplified
*       1.0  (18-Mar-2017) First release
*       0.9x (XX-Jan-2017) Review complete file...
*       0.95 (14-Sep-2016) Reviewed comments and .rfx format
*       0.9  (12-Sep-2016) Defined WaveParams struct and command line functionality
*       0.8  (09-Sep-2016) Added open/save file dialogs using tinyfiledialogs library
*       0.7  (04-Sep-2016) Program variables renaming for consistency, code reorganized
*       0.6  (30-Aug-2016) Interface redesigned (reduced size) and new features added (wave drawing)
*       0.5  (27-Aug-2016) Completed port and adaptation from sfxr (only sound generation and playing)
*
*   DEPENDENCIES:
*       raylib 3.0              - Windowing/input management and drawing.
*       raygui 2.7              - Immediate-mode GUI controls.
*       tinyfiledialogs 3.4.3   - Open/save file dialogs, it requires linkage with comdlg32 and ole32 libs.
*
*   COMPILATION (Windows - MinGW):
*       gcc -o rfxgen.exe rfxgen.c external/tinyfiledialogs.c -s rfxgen_icon -Iexternal /
*           -lraylib -lopengl32 -lgdi32 -lcomdlg32 -lole32 -std=c99 -Wl,--subsystem,windows
*
*   COMPILATION (Linux - GCC):
*       gcc -o rfxgen rfxgen.c external/tinyfiledialogs.c -s -Iexternal -no-pie -D_DEFAULT_SOURCE /
*           -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
*
*   NOTE: On PLATFORM_ANDROID and PLATFORM_WEB file dialogs are not available
*
*   DEVELOPERS:
*       Ramon Santamaria (@raysan5):   Developer, supervisor, updater and maintainer.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2020 raylib technologies (@raylibtech).
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include <math.h>		// Required for: sinf(), pow()
#include <stdbool.h>
#include <stdio.h>		// Required for: FILE, fopen(), fread(), fwrite(), ftell(), fseek() fclose()
#include <stdlib.h>		// Required for: calloc(), free()
#include <string.h>		// Required for: strcmp()

#include <rfxgen.h>

#define PI 3.14159265358979323846

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Wave parameters type (96 bytes)
struct WaveParams
{
    // Random seed used to generate the wave
    int randSeed;

    // Wave type (square, sawtooth, sine, noise)
    int waveTypeValue;

    // Wave envelope parameters
    float attackTimeValue;
    float sustainTimeValue;
    float sustainPunchValue;
    float decayTimeValue;

    // Frequency parameters
    float startFrequencyValue;
    float minFrequencyValue;
    float slideValue;
    float deltaSlideValue;
    float vibratoDepthValue;
    float vibratoSpeedValue;
    //float vibratoPhaseDelayValue;

    // Tone change parameters
    float changeAmountValue;
    float changeSpeedValue;

    // Square wave parameters
    float squareDutyValue;
    float dutySweepValue;

    // Repeat parameters
    float repeatSpeedValue;

    // Phaser parameters
    float phaserOffsetValue;
    float phaserSweepValue;

    // Filter parameters
    float lpfCutoffValue;
    float lpfCutoffSweepValue;
    float lpfResonanceValue;
    float hpfCutoffValue;
    float hpfCutoffSweepValue;
};

// Returns a random value between min and max (both included)
static int GetRandomValue(int min, int max)
{
	if (min > max)
	{
		int tmp = max;
		max = min;
		min = tmp;
	}

	return (rand()%(abs(max - min) + 1) + min);
}

// Generates new wave from wave parameters
// NOTE: By default wave is generated as 44100Hz, 32bit float, mono
Wave GenerateWave(WaveParams *params)
{
    // NOTE: GetRandomValue() is provided by raylib and seed is initialized at InitWindow()
    #define GetRandomFloat(range) ((float)GetRandomValue(0, 10000)/10000.0f*range)

    if (params->randSeed != 0) srand(params->randSeed);   // Initialize seed if required

    // Configuration parameters for generation
    // NOTE: Those parameters are calculated from selected values
    int phase = 0;
    double fperiod = 0.0;
    double fmaxperiod = 0.0;
    double fslide = 0.0;
    double fdslide = 0.0;
    int period = 0;
    float squareDuty = 0.0f;
    float squareSlide = 0.0f;
    int envelopeStage = 0;
    int envelopeTime = 0;
    int envelopeLength[3] = { 0 };
    float envelopeVolume = 0.0f;
    float fphase = 0.0f;
    float fdphase = 0.0f;
    int iphase = 0;
    float phaserBuffer[1024] = { 0.0f };
    int ipp = 0;
    float noiseBuffer[32] = { 0.0f };       // Required for noise wave, depends on random seed!
    float fltp = 0.0f;
    float fltdp = 0.0f;
    float fltw = 0.0f;
    float fltwd = 0.0f;
    float fltdmp = 0.0f;
    float fltphp = 0.0f;
    float flthp = 0.0f;
    float flthpd = 0.0f;
    float vibratoPhase = 0.0f;
    float vibratoSpeed = 0.0f;
    float vibratoAmplitude = 0.0f;
    int repeatTime = 0;
    int repeatLimit = 0;
    int arpeggioTime = 0;
    int arpeggioLimit = 0;
    double arpeggioModulation = 0.0;

    // HACK: Security check to avoid crash (why?)
    if (params->minFrequencyValue > params->startFrequencyValue) params->minFrequencyValue = params->startFrequencyValue;
    if (params->slideValue < params->deltaSlideValue) params->slideValue = params->deltaSlideValue;

    // Reset sample parameters
    //----------------------------------------------------------------------------------------
    fperiod = 100.0/(params->startFrequencyValue*params->startFrequencyValue + 0.001);
    period = (int)fperiod;
    fmaxperiod = 100.0/(params->minFrequencyValue*params->minFrequencyValue + 0.001);
    fslide = 1.0 - pow((double)params->slideValue, 3.0)*0.01;
    fdslide = -pow((double)params->deltaSlideValue, 3.0)*0.000001;
    squareDuty = 0.5f - params->squareDutyValue*0.5f;
    squareSlide = -params->dutySweepValue*0.00005f;

    if (params->changeAmountValue >= 0.0f) arpeggioModulation = 1.0 - pow((double)params->changeAmountValue, 2.0)*0.9;
    else arpeggioModulation = 1.0 + pow((double)params->changeAmountValue, 2.0)*10.0;

    arpeggioLimit = (int)(pow(1.0f - params->changeSpeedValue, 2.0f)*20000 + 32);

    if (params->changeSpeedValue == 1.0f) arpeggioLimit = 0;     // WATCH OUT: float comparison

    // Reset filter parameters
    fltw = pow(params->lpfCutoffValue, 3.0f)*0.1f;
    fltwd = 1.0f + params->lpfCutoffSweepValue*0.0001f;
    fltdmp = 5.0f/(1.0f + pow(params->lpfResonanceValue, 2.0f)*20.0f)*(0.01f + fltw);
    if (fltdmp > 0.8f) fltdmp = 0.8f;
    flthp = pow(params->hpfCutoffValue, 2.0f)*0.1f;
    flthpd = 1.0 + params->hpfCutoffSweepValue*0.0003f;

    // Reset vibrato
    vibratoSpeed = pow(params->vibratoSpeedValue, 2.0f)*0.01f;
    vibratoAmplitude = params->vibratoDepthValue*0.5f;

    // Reset envelope
    envelopeLength[0] = (int)(params->attackTimeValue*params->attackTimeValue*100000.0f);
    envelopeLength[1] = (int)(params->sustainTimeValue*params->sustainTimeValue*100000.0f);
    envelopeLength[2] = (int)(params->decayTimeValue*params->decayTimeValue*100000.0f);

    fphase = pow(params->phaserOffsetValue, 2.0f)*1020.0f;
    if (params->phaserOffsetValue < 0.0f) fphase = -fphase;

    fdphase = pow(params->phaserSweepValue, 2.0f)*1.0f;
    if (params->phaserSweepValue < 0.0f) fdphase = -fdphase;

    iphase = abs((int)fphase);

    for (int i = 0; i < 32; i++) noiseBuffer[i] = GetRandomFloat(2.0f) - 1.0f;      // WATCH OUT: GetRandomFloat()

    repeatLimit = (int)(pow(1.0f - params->repeatSpeedValue, 2.0f)*20000 + 32);

    if (params->repeatSpeedValue == 0.0f) repeatLimit = 0;
    //----------------------------------------------------------------------------------------

    // NOTE: We reserve enough space for up to 10 seconds of wave audio at given sample rate
    // By default we use float size samples, they are converted to desired sample size at the end
    float *buffer = calloc(MAX_WAVE_LENGTH_SECONDS*WAVE_SAMPLE_RATE, sizeof(float));
    bool generatingSample = true;
    int sampleCount = 0;

    for (int i = 0; i < MAX_WAVE_LENGTH_SECONDS*WAVE_SAMPLE_RATE; i++)
    {
        if (!generatingSample)
        {
            sampleCount = i;
            break;
        }

        // Generate sample using selected parameters
        //------------------------------------------------------------------------------------
        repeatTime++;

        if ((repeatLimit != 0) && (repeatTime >= repeatLimit))
        {
            // Reset sample parameters (only some of them)
            repeatTime = 0;

            fperiod = 100.0/(params->startFrequencyValue*params->startFrequencyValue + 0.001);
            period = (int)fperiod;
            fmaxperiod = 100.0/(params->minFrequencyValue*params->minFrequencyValue + 0.001);
            fslide = 1.0 - pow((double)params->slideValue, 3.0)*0.01;
            fdslide = -pow((double)params->deltaSlideValue, 3.0)*0.000001;
            squareDuty = 0.5f - params->squareDutyValue*0.5f;
            squareSlide = -params->dutySweepValue*0.00005f;

            if (params->changeAmountValue >= 0.0f) arpeggioModulation = 1.0 - pow((double)params->changeAmountValue, 2.0)*0.9;
            else arpeggioModulation = 1.0 + pow((double)params->changeAmountValue, 2.0)*10.0;

            arpeggioTime = 0;
            arpeggioLimit = (int)(pow(1.0f - params->changeSpeedValue, 2.0f)*20000 + 32);

            if (params->changeSpeedValue == 1.0f) arpeggioLimit = 0;     // WATCH OUT: float comparison
        }

        // Frequency envelopes/arpeggios
        arpeggioTime++;

        if ((arpeggioLimit != 0) && (arpeggioTime >= arpeggioLimit))
        {
            arpeggioLimit = 0;
            fperiod *= arpeggioModulation;
        }

        fslide += fdslide;
        fperiod *= fslide;

        if (fperiod > fmaxperiod)
        {
            fperiod = fmaxperiod;

            if (params->minFrequencyValue > 0.0f) generatingSample = false;
        }

        float rfperiod = fperiod;

        if (vibratoAmplitude > 0.0f)
        {
            vibratoPhase += vibratoSpeed;
            rfperiod = fperiod*(1.0 + sinf(vibratoPhase)*vibratoAmplitude);
        }

        period = (int)rfperiod;

        if (period < 8) period=8;

        squareDuty += squareSlide;

        if (squareDuty < 0.0f) squareDuty = 0.0f;
        if (squareDuty > 0.5f) squareDuty = 0.5f;

        // Volume envelope
        envelopeTime++;

        if (envelopeTime > envelopeLength[envelopeStage])
        {
            envelopeTime = 0;
            envelopeStage++;

            if (envelopeStage == 3) generatingSample = false;
        }

        if (envelopeStage == 0) envelopeVolume = (float)envelopeTime/envelopeLength[0];
        if (envelopeStage == 1) envelopeVolume = 1.0f + pow(1.0f - (float)envelopeTime/envelopeLength[1], 1.0f)*2.0f*params->sustainPunchValue;
        if (envelopeStage == 2) envelopeVolume = 1.0f - (float)envelopeTime/envelopeLength[2];

        // Phaser step
        fphase += fdphase;
        iphase = abs((int)fphase);

        if (iphase > 1023) iphase = 1023;

        if (flthpd != 0.0f)     // WATCH OUT!
        {
            flthp *= flthpd;
            if (flthp < 0.00001f) flthp = 0.00001f;
            if (flthp > 0.1f) flthp = 0.1f;
        }

        float ssample = 0.0f;

        #define MAX_SUPERSAMPLING   8

        // Supersampling x8
        for (int si = 0; si < MAX_SUPERSAMPLING; si++)
        {
            float sample = 0.0f;
            phase++;

            if (phase >= period)
            {
                //phase = 0;
                phase %= period;

                if (params->waveTypeValue == 3)
                {
                    for (int i = 0;i < 32; i++) noiseBuffer[i] = GetRandomFloat(2.0f) - 1.0f;   // WATCH OUT: GetRandomFloat()
                }
            }

            // base waveform
            float fp = (float)phase/period;

            switch (params->waveTypeValue)
            {
                case 0: // Square wave
                {
                    if (fp < squareDuty) sample = 0.5f;
                    else sample = -0.5f;

                } break;
                case 1: sample = 1.0f - fp*2; break;    // Sawtooth wave
                case 2: sample = sinf(fp*2*PI); break;  // Sine wave
                case 3: sample = noiseBuffer[phase*32/period]; break; // Noise wave
                default: break;
            }

            // LP filter
            float pp = fltp;
            fltw *= fltwd;

            if (fltw < 0.0f) fltw = 0.0f;
            if (fltw > 0.1f) fltw = 0.1f;

            if (params->lpfCutoffValue != 1.0f)  // WATCH OUT!
            {
                fltdp += (sample-fltp)*fltw;
                fltdp -= fltdp*fltdmp;
            }
            else
            {
                fltp = sample;
                fltdp = 0.0f;
            }

            fltp += fltdp;

            // HP filter
            fltphp += fltp - pp;
            fltphp -= fltphp*flthp;
            sample = fltphp;

            // Phaser
            phaserBuffer[ipp & 1023] = sample;
            sample += phaserBuffer[(ipp - iphase + 1024) & 1023];
            ipp = (ipp + 1) & 1023;

            // Final accumulation and envelope application
            ssample += sample*envelopeVolume;
        }

        #define SAMPLE_SCALE_COEFICIENT 0.2f    // NOTE: Used to scale sample value to [-1..1]

        ssample = (ssample/MAX_SUPERSAMPLING)*SAMPLE_SCALE_COEFICIENT;
        //------------------------------------------------------------------------------------

        // Accumulate samples in the buffer
        if (ssample > 1.0f) ssample = 1.0f;
        if (ssample < -1.0f) ssample = -1.0f;

        buffer[i] = ssample;
    }

    Wave genWave;
    genWave.sampleCount = sampleCount;
    genWave.sampleRate = WAVE_SAMPLE_RATE; // By default 44100 Hz
    genWave.sampleSize = 32;               // By default 32 bit float samples
    genWave.channels = 1;                  // By default 1 channel (mono)

    genWave.data = calloc(genWave.sampleCount*genWave.channels, genWave.sampleSize/8);
    memcpy(genWave.data, buffer, genWave.sampleCount*genWave.channels*genWave.sampleSize/8);

    free(buffer);

    return genWave;
}

// Load .rfx (rFXGen) sound parameters file
WaveParams *LoadWaveParams(const char *fileName)
{
    WaveParams *params = malloc(sizeof(WaveParams));
	FILE *rfxFile;
	char signature[4];
	unsigned short version, length;

	if(params == NULL)
		goto out;

	rfxFile = fopen(fileName, "rb");
	if (rfxFile == NULL)
		goto out;

	// Fx Sound File Structure (.rfx)
	// ------------------------------------------------------
	// Offset | Size  | Type       | Description
	// ------------------------------------------------------
	// 0      | 4     | char       | Signature: "rFX "
	// 4      | 2     | short      | Version: 200
	// 6      | 2     | short      | Data length: 96 bytes
	// 8      | 96    | WaveParams | Wave parameters
	// ------------------------------------------------------

	// Read .rfx file header
	fread(signature, 4, sizeof(char), rfxFile);
	if(strncmp(signature, "rFX ", 4) != 0)
	{
		printf("[%s] rFX file does not seem to be valid\n", fileName);
		fclose(rfxFile);
		goto out;
	}

	fread(&version, 1, sizeof(unsigned short), rfxFile);
	fread(&length, 1, sizeof(unsigned short), rfxFile);

	if (version != 200)
		printf("[%s] rFX file version not supported (%i)\n", fileName, version);
	else if (length != sizeof(WaveParams))
		printf("[%s] Wrong rFX wave parameters size\n", fileName);
	else
		fread(params, 1, sizeof(WaveParams), rfxFile);   // Load wave generation parameters

	fclose(rfxFile);

out:
	return params;
}
