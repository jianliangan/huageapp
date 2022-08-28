//
// Created by ajl <429200247@qq.com> on 2020/12/20.
//loop负责采集接受
//

#ifndef ANDROID_PROJECT_VIDEOHANDLE_H
#define ANDROID_PROJECT_VIDEOHANDLE_H

#include <string>

#include "../net/protocol/core/tools.h"
#include "../net/protocol/huage_sendstream.h"
#include "../net/protocol/huage_recvstream.h"
#include "video_source.h"
#include "handle_com.h"


class VideoHandle {
public:
    bool finished=false;
//对端地址，考虑放到别处
    HandleCom handle;
    void *vManDec= nullptr;
    void *vEncoder= nullptr;
    VideoSource *vSource=nullptr;
    uint16_t lasttime;
    FILE *f2;
    static void RecvCallback(void *ctx,media_frame_chain *mfc, HgSessionC *sess);

    ////////////////////////
   void SendData(void *output, uint32_t size,uint16_t time);
    //////////////////////////////
    VideoHandle();
    ~VideoHandle();
    //接收
};




#endif //ANDROID_PROJECT_videoHandle_H