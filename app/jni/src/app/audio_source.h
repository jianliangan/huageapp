//
// Created by ajl on 2021/1/18.
//

#ifndef ANDROID_PROJECT_AUDIOSOURCE_H
#define ANDROID_PROJECT_AUDIOSOURCE_H

#define ADEC_PCMFRAME_MAXBUF 4096
#include "../net/protocol/core/tools.h"
class AudioSource {
public:

    bool haslog;
    bool finished;
int channels;
int freq;
int bits_per_sample;
    FILE *f2;
    void *encoderCtx;
    void (*toEncoCallback)(void *ctx,uint8_t *timesPcmdata,uint32_t size)= nullptr;
    AudioSource();

    int CaptureStart();

    int Loop();
    void QueueOutAudio(uint8_t *data, uint32_t size);
    static void Callback(void *udata, uint8_t *stream, uint32_t len);

    static void *ConsumeQueue(void *ctx);

};


#endif //ANDROID_PROJECT_AUDIO_SOURCE_H
