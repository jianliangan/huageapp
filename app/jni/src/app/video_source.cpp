//
// Created by ajl on 2021/1/18.
//

#include "video_source.h"
#include "../net/protocol/core/tools.h"
#include "app.h"

VideoSource::VideoSource() {
    finished=false;
    initRendered=false;

}

void VideoSource::Callback(void *udata, uint8_t *stream, uint32_t len) {
    //ALOGI(0, "ccccccccccccccccccccccccc %d", len);
}

void VideoSource::RenderVideo(int width,int height,uint8_t *ydata,uint8_t *udata,uint8_t *vdata,int ystride,int uvstride,int flag) {

    RenderInternal(width,height,ydata,udata,vdata,ystride,uvstride,flag);
}
void *VideoSource::Render(void *data) {

    return 0;
}

void VideoSource::InitRender(int cameraw, int camerah){

}

void VideoSource::VideoCapture(yuvchar planey, int pitchy, yuvchar planeu, int pitchu, yuvchar planev, int pitchv,
                  int ulength, int pixpitchy, int pixpitchuv, int cameraw, int camerah){

}
void VideoSource::RenderInternal(int width,int height,uint8_t *ydata,uint8_t *udata,uint8_t *vdata,int ystride,int uvstride,int flag){

}