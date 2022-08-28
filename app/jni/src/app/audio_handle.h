//
// Created by ajl <429200247@qq.com> on 2020/12/20.
//loop负责采集接受
//

#ifndef ANDROID_PROJECT_AUDIOHANDLE_H
#define ANDROID_PROJECT_AUDIOHANDLE_H
#include <string>
#include "../net/protocol/core/tools.h"

#include "../net/protocol/huage_sendstream.h"
#include "../net/protocol/huage_recvstream.h"
#include "audio_decoder.h"
#include "audio_encoder.h"
#include "audio_source.h"
#include "handle_com.h"

class AudioHandle {
public:
    bool finished;
    HandleCom handle;
    //对端地址，考虑放到别处
    AudioDecoder *aDecoder= nullptr;


    AudioEncoder *aEncoder= nullptr;

    FILE *f2;
    AudioHandle(audio_config_t *recconfig,audio_config_t *playconfig);
    ~AudioHandle();
    void Start();

    void SendData(uint8_t *output, uint32_t size, uint16_t time,int comptype);
    int RunAudioEncoder();
    int RunAudioDecoder();

    static void RecvCallback(void *ctx,media_frame_chain *mfc,HgSessionC *sess);


    //接收
};
#endif //ANDROID_PROJECT_audioHandle_H