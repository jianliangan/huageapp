//
// Created by ajl on 2021/1/18.
//

#ifndef ANDROID_PROJECT_APP_H
#define ANDROID_PROJECT_APP_H
#include "../net/protocol/core/tools.h"
#include "audio_source.h"
#include "audio_encoder.h"
#include "audio_decoder.h"
#include "audio_handle.h"
#include "text_handle.h"
#include "video_handle.h"

#include "net/protocol/rtp_packet.h"

#include "../base/json.hpp"
#include "../net/threads/hg_channel.h"
#include "../ex_hgmain.h"
#include "room.h"

inline void reqJsoHead(uint32_t uniqid,std::string uri,nlohmann::json &jss,nlohmann::json const &body){
    jss["v"]= 0;
    jss["i"]= uniqid ;
    jss["c"]=uri;
    jss["req"]=body;
}


class HgClient;
enum dataType{
    ISAUDIO,ISVIDEO
};
class App {
public:
    App();
    uint32_t startts;
    bool islittleend;
    HgChannel Channel;
    uint32_t ssrc;
    char uname[10];
    uint32_t roomid;
    char *serverip;
    int serverport;
    uint8_t *loginBuf;


    HgClient *hgcc;

    AudioHandle *aHandle;
    TextHandle *tHandle;

    pthread_mutex_t syncEventmtx;

    VideoHandle *vHandle;
    SessionC *huageConn;
    uint32_t loginNum;
    void DoLogin();

    int CreateEnterRoom(int roomid,SessionC *sess);

    static onLogined onLogin_Callback;
    static onRecvData onRecvVieData_Callback;
    static onRecvDataA onRecvAudData_Callback;
};

extern App *hgapp;
#endif //ANDROID_PROJECT_APP_H
