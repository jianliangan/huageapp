//
// Created by ajl on 2021/6/16.
//

#include "audio_recorder.h"
#include "audio_player.h"
#include "../include/MySdl.h"
#include "audio_common.h"

typedef struct EchoAudioEngine {
    SLmilliHertz fastPathSampleRate_;
    uint32_t fastPathFramesPerBuf_;
    uint16_t sampleChannels_;
    uint16_t bitsPerSample_;

    SLObjectItf slEngineObj_;
    SLEngineItf slEngineItf_;

    AudioRecorder *recorder_ = nullptr;
    AudioPlayer *player_ = nullptr;
    EchoAudioEngine():fastPathFramesPerBuf_(0),sampleChannels_(0),bitsPerSample_(0){};
} EchoAudioEngine;
static EchoAudioEngine engine;

static int64_t tmp=0;
void writeBufToPlay(uint8_t *data, int size,int pts) {
    if (engine.player_ != nullptr && engine.player_->isfinish) {

        int64_t aaa=hgetSysTimeMicros()/1000-tmp;
         if(aaa>20)
         ALOGI(0,"time11111----:%lld",aaa);
        tmp=hgetSysTimeMicros()/1000;

        engine.player_->writeBuf(data, size);
        if (!engine.player_->isplaying) {
            engine.player_->Start();
            engine.player_->isplaying = true;
        }
    }

}

void myApiCreate(JNI_AUDIO_CALLBACK reccb, JNI_AUDIO_CALLBACK placb) {

    SLresult result;
    memset(&engine, 0, sizeof(engine));
    result = slCreateEngine(&engine.slEngineObj_, 0, NULL, 0, NULL, NULL);
    result = (*engine.slEngineObj_)->Realize(engine.slEngineObj_, SL_BOOLEAN_FALSE);
    result = (*engine.slEngineObj_)->GetInterface(engine.slEngineObj_, SL_IID_ENGINE,
                                                  &engine.slEngineItf_);
    //***************************recoder
    int bufSize1;
//将来输出24位的音频，需要可以动态设置24，16位输出，就是解码那需要改，24位是服务器合成的
    bufSize1 = ((CONF_CAPTURE_FRAMEPERBUF * CONF_CAPTURE_DEEP + 7) >> 3) * CONF_REND_CAPTURE_CHANNELS;

    SampleFormat sampleFormat;
    memset(&sampleFormat, 0, sizeof(sampleFormat));
    //SL_PCMSAMPLEFORMAT_FIXED_16

    sampleFormat.channels = CONF_REND_CAPTURE_CHANNELS;
    sampleFormat.sampleRate_ = CONF_CAPTURE_SAMPLESRATE;
    sampleFormat.representation_ = CONF_CAPTURE_ANDROID_DATAFORMAT_PCM;//具体怎么选看audio_common.cpp
    engine.recorder_ = new AudioRecorder(&sampleFormat, engine.slEngineItf_);

    engine.recorder_->createBuffer(DEVICE_RECODE_BUFFER_QUEUE_LEN, bufSize1);
    engine.recorder_->RegisterCallback(reccb, engine.recorder_);



    //*************************player
    int bufSize2;
    bufSize2 = ((CONF_PLAY_FRAMEPERBUF * CONF_PLAY_DEEP + 7) >> 3) * CONF_REND_PLAY_CHANNELS;
    SampleFormat sampleFormat2;
    memset(&sampleFormat2, 0, sizeof(sampleFormat2));
    //SL_PCMSAMPLEFORMAT_FIXED_16

    sampleFormat2.channels = CONF_REND_PLAY_CHANNELS;
    sampleFormat2.representation_ = CONF_PLAY_ANDROID_DATAFORMAT_PCM;
    sampleFormat2.sampleRate_ = CONF_PLAY_SAMPLESRATE;
    engine.player_ = new AudioPlayer(&sampleFormat2, engine.slEngineItf_);
    assert(engine.player_);
    engine.player_->createBuffer(DEVICE_PLAY_BUFFER_QUEUE_LEN, bufSize2);
    engine.player_->RegisterCallback(placb, engine.player_);

    engine.recorder_->Start();
}