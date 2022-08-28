/*
  Copyright (C) 1997-2020 ajl <429200247@qq.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/*
 * https://github.com/kueblert/AndroidMediaCodec/blob/master/nativecodecvideo.cpp   native media
 * http://anddymao.com/2019/10/16/2019-10-16-ndk-MediaCodec/   native media
 *
需要优化的问题：
1、play和record需要加缓冲，解决写快读慢的问题，会有杂音
 */

#include <stdlib.h>

#include "app/audio_handle.h"
#include "app/audio_source.h"
#include "app/app.h"
#include "net/protocol/core/tools.h"

#include "ex_hgaudio_common.h"
#include "net/hg_client.h"
#include "h264parse/h264_stream.h"

static void clientCB(void *pth, void *ctx, void *params, int psize);
static std::atomic<uint16_t> videopts;
static int get_slice_type(bs_t *b) {
    bs_read_ue(b);
    return bs_read_ue(b);
}

void appInitProperty(onLogined f0, onRecvData f1, onRecvDataA f2) {
    App::onLogin_Callback = f0;
    App::onRecvVieData_Callback = f1;
    App::onRecvAudData_Callback = f2;
}

int appAudioFramtoEnco(ev_audio_t *eat) {
//ajltest
//return 0;
    hgapp->aHandle->aEncoder->PushDataIn(eat,videopts);
    return 0;
}

int appSendVideo(uint8_t *output, uint32_t size, int times, int channel) {
    videopts=(uint16_t)times;
//return 0;
    char aaa[]="yyyyyyyyyyyyyyyy 1 1 3yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
    FILE *f2 = fopen(APP_ROOT_"send0.data", "a+");
    fwrite(output, 1, size, f2);
    fwrite(aaa,1,strlen(aaa),f2);
    fclose(f2);

    uint8_t *nalstart=nullptr;
    int rbspsize;
    int naltype=h264_get_type(output,size,&nalstart,&rbspsize);
    if(naltype==7||naltype==8)
    {
        sps_t sps;
        int ret=h264_get_attr_sps(nalstart,rbspsize,&sps);
        if(ret<0){
            return 0;
        }
        int width=0;
        int height=0;
        int fps=0;
        h264_get_sps_widthheight(&sps,&width,&height,&fps);

        ALOGI(0, "aaaaaaaaaaaaaaaaaaaaaaaPic width:%d,  height:%d,  fps:%d",&width,&height,&fps);
    }


    hgapp->vHandle->SendData(output, size, times);
    return 0;

}

int appCreateEnterRoom(int roomid) {
    return hgapp->CreateEnterRoom(roomid, hgapp->huageConn);
}



int appLogin(int argc, myjchar*argv) {

    if (argc != 4) {
        ALOGI(0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!参数错误\n");
    }

    char *roomid = argv[0].data;
    char *userid = argv[1].data;
    char *server = argv[2].data;
    char *port = argv[3].data;
    char *uname=argv[4].data;
    int unamesiz=argv[4].size;
    bool islittleend = checkLitEndian() == 1 ? true : false;


    hgapp->islittleend = islittleend;
    hgapp->roomid = (uint32_t)(atoll((char *) roomid));
    hgapp->ssrc = (uint32_t)(atoll((char *) userid));
    hgapp->serverip = (char *) server;
    hgapp->serverport = atoi((char *) port);
    memcpy(hgapp->uname,uname,unamesiz);

    hgapp->tHandle = new TextHandle();
    if (hgapp->hgcc == nullptr) {
        hgapp->hgcc = new HgClient(hgapp->serverip, hgapp->serverport);

        hgapp->hgcc->recvaudioCallback = AudioHandle::RecvCallback;
        hgapp->hgcc->recvvideoCallback = VideoHandle::RecvCallback;

        hgapp->hgcc->recvtextCallback = TextHandle::RecvCallback;
        hgapp->hgcc->verify_auth = TextHandle::Verify_auth;

        hgapp->hgcc->asyncLogin = TextHandle::AsyncLogin;

        hgapp->hgcc->textctx = hgapp->tHandle;

        hgapp->hgcc->hnab->selfcb.chan = &hgapp->Channel;
        hgapp->hgcc->hnab->selfcb.cbhandle = clientCB;
        hgapp->hgcc->StartRun();
    }
    hgapp->DoLogin();

    return 0;
}

void clientCB(void *pth, void *ctx, void *params, int psize) {
    res_params_s *sps = (res_params_s *) params;

}

int
appInitStream(audio_config_t *recconfig, audio_config_t *playconfig) {
    hgapp->aHandle = new AudioHandle(recconfig, playconfig);
    hgapp->vHandle = new VideoHandle();
    hgapp->hgcc->videoctx = hgapp->vHandle;
    hgapp->hgcc->audioctx = hgapp->aHandle;
    hgapp->aHandle->Start();

    /* stop playing back, quit. */
    return 0;
}

int appMainThRun() {
    hgapp = new App();
    init16Num();
    while (true) {
        hgapp->Channel.Drive(nullptr, 1);
    }

    return 0;
}
