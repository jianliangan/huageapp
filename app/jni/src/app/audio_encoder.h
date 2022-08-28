//
// Created by ajl on 2021/9/2.
//

#ifndef HUAGERTP_APP_AUDIO_ENCODER_H
#define HUAGERTP_APP_AUDIO_ENCODER_H

#include "../net/protocol/rtp_packet.h"
#include "../net/protocol/core/hg_queue.h"
#include "../net/protocol/core/tools.h"
#include "../base/g711.h"
#include "audio_common.h"



class free_frame_a
{
public:
    free_frame_a *next;
    char *data;
    int size;
    void init(){
        next= nullptr;
        data= nullptr;
    }
};
typedef struct audio_frame_raw_pa {
    free_frame_a *data;
    uint16_t apts;
    uint16_t vpts;
} audio_frame_raw_pa;
class AudioEncoder{
public:
    //myQueue *eventQueue;
    //sem_t eventSem;
    audio_config_t captureconf;
    free_frame_a *framefreea=nullptr;
    uint16_t lasttime;
    int framerawsize;
    ev_audio_t audioev0;
    uint8_t audioev_buf[EV_BUF_PTR_LENGTH];
   // pthread_mutex_t eventMtx;
    int input_size;
    bool finished;
    //pthread_t eventThread;
    AudioEncoder(audio_config_t *recconfig);
    void PushDataIn(ev_audio_t *event,uint16_t vpts);

    int InitEncoder();

    static void HandleEncoder(void *pth, void *ctx, void *params, int psize);

    free_frame_a *GetFreeFrame();
    static void FreeByChan(void *pth, void *ctx, void *params, int psize);
    static void WriteFree(void *ctx, void *data);
};
#endif //HUAGERTP_APP_AUDIO_ENCODER_H
