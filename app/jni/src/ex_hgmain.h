//
// Created by ajl on 2021/7/5.
//

#ifndef HUAGERTP_APP_MAIN_H
#define HUAGERTP_APP_MAIN_H
#include "ex_hgaudio_common.h"
#include "net/protocol/core/hg_buf_comm.h"
#define MAIN_DLL_EXPORT __attribute__((visibility("default")))
typedef struct myjchar{
    char *data;
    int size;
} myjchar;
typedef void (*onLogined)() ;
typedef void (*onRecvData)(int type,hg_chain_node* data,int size,int pts) ;
typedef void (*onRecvDataA)(int type,uint8_t* data,int size,int pts) ;
MAIN_DLL_EXPORT int appLogin(int argc, myjchar*argv);
MAIN_DLL_EXPORT int appInitStream(audio_config_t *recconfig,audio_config_t *playconfig);
MAIN_DLL_EXPORT int appCreateEnterRoom(int roomid);

MAIN_DLL_EXPORT int appMainThRun();
MAIN_DLL_EXPORT int appAudioFramtoEnco(ev_audio_t *eat);
MAIN_DLL_EXPORT int appSendVideo(uint8_t *output, uint32_t size, int times,int channel);
MAIN_DLL_EXPORT void appInitProperty(onLogined f0,onRecvData f1,onRecvDataA f2);
#endif //HUAGERTP_APP_MAIN_H
