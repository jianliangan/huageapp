//
// Created by ajl on 2021/6/23.
//

#ifndef HUAGERTP_APP_AUDIO_PLAYER_H
#define HUAGERTP_APP_AUDIO_PLAYER_H

#include <sys/types.h>
#include "audio_common.h"

typedef struct playSample {
    uint8_t *data;
    playSample():data(nullptr){};
} playSample;

class AudioPlayer {
    int bufSize = 0;
    int currentsize=0;
    //int64_t tmp=0;
    SLObjectItf outputMixObjectItf;
    SLObjectItf playerObjectItf;
    SLPlayItf playItf_o;
    std::atomic<uint32_t> produceCount;
    std::atomic<uint32_t> consumeCount;
    SLAndroidSimpleBufferQueueItf playBufferQueueItf;
    playSample **buffer_o = nullptr;

    JNI_AUDIO_CALLBACK callback_f;

public:
    bool isplaying = false;
    bool isfinish=false;
    explicit AudioPlayer(SampleFormat *sampleFormat, SLEngineItf engine);

    ~AudioPlayer();

    SLresult Start(void);

    void Stop(void);

    void writeBuf(uint8_t *buf, int size);

    void createBuffer(int len, int size);

    static void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx);

    void RegisterCallback(JNI_AUDIO_CALLBACK cb, void *ctx);
};

#endif //HUAGERTP_APP_AUDIO_PLAYER_H
