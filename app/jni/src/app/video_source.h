//
// Created by ajl on 2021/1/18.
//

#ifndef ANDROID_PROJECT_VIDEOSOURCE_H
#define ANDROID_PROJECT_VIDEOSOURCE_H

#include "../net/protocol/core/tools.h"
#include "../net/protocol/core/cctools.hpp"
#include "av_internal.h"
#define YUVMAXWIDTH 2000
#define YUVMAXHEIGHT 2000
#define MYROTATE 270

typedef struct renderEvent{
    int ssrc;
    renderEvent * next;
} renderEvent;

typedef struct cameraInfo {
    int w;
    int h;
} cameraInfo;

class VideoSource {
public:
    pthread_t pthRender;
    cameraInfo camerainfo;
    uint8_t *timCameraPic=nullptr;
    uint8_t *yuvData[3];

    uint8_t *cameraPic=nullptr;
    int cameraPicLen=0;
    int vWidth=0;
    int vHeight=0;
    std::atomic_bool finished;
    bool initRendered;
    void *encoderCtx= nullptr;

    void (*toEncoCallback)(void *ctx, uint8_t *timesYuvdata, uint32_t size)= nullptr;

    VideoSource();

    void RenderVideo(int width, int height, uint8_t *ydata, uint8_t *udata, uint8_t *vdata,int ystride,int uvstride,int flag);

    void
    VideoCapture(yuvchar planey, int pitchy, yuvchar planeu, int pitchu, yuvchar planev, int pitchv,
                 int ulength, int pixpitchy, int pixpitchuv, int cameraw, int camerah);


    void RenderInternal(int width,int height,uint8_t *ydata,uint8_t *udata,uint8_t *vdata,int ystride,int uvstride,int flag);

    void InitRender(int cameraw, int camerah);

    static void Callback(void *udata, uint8_t *stream, uint32_t len);

    static void *Render(void *data);
};

#endif //ANDROID_PROJECT_VIDEO_SOURCE_H
