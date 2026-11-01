#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // xd

static ndspWaveBuf waveBuf;
static s16 *audioData;
static size_t audioSize;

void audio_init(void) {
    ndspInit();

    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    ndspChnReset(0);
    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, 32000);
    ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

    float mix[12] = {0};
    mix[0] = 1.0f; // Left
    mix[1] = 1.0f; // Right

    ndspChnSetMix(0, mix);
}

bool audio_play(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) return false;

    fseek(file, 0, SEEK_END);
    audioSize = ftell(file);
    rewind(file);

    if (audioSize == 0 || audioSize % 4 != 0) {
        fclose(file);
        return false;
    }

    audioData = linearAlloc(audioSize);

    if (!audioData) {
        fclose(file);
        return false;
    }

    if (fread(audioData, 1, audioSize, file) != audioSize) {
        linearFree(audioData);
        audioData = NULL;
        fclose(file);
        return false;
    }

    fclose(file);

    DSP_FlushDataCache(audioData, audioSize);

    memset(&waveBuf, 0, sizeof(waveBuf));

    waveBuf.data_pcm16 = audioData;
    waveBuf.nsamples = audioSize / 4;
    waveBuf.looping = true;

    ndspChnWaveBufAdd(0, &waveBuf);

    return true;
}

void audio_stop(void) {
    ndspChnWaveBufClear(0);
	while (waveBuf.status != NDSP_WBUF_DONE && waveBuf.status != NDSP_WBUF_FREE)
	        svcSleepThread(1000000);
    if (audioData) {
        linearFree(audioData);
        audioData = NULL;
    }
}
