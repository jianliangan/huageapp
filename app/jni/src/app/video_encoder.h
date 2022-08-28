//
// Created by ajl on 2021/1/17.
//

#ifndef ANDROID_PROJECT_VIDEOENCODER_H
#define ANDROID_PROJECT_VIDEOENCODER_H
#include "../net/protocol/core/tools.h"
#define TIMEOUT_USEC 2000
#ifdef USE_OPENH264_ENC

#include "measure_time.h"
#include "typedefs.h"
#include "codec_def.h"
#include "codec_api.h"
#include "extern.h"
#include "macros.h"
#include "wels_const.h"
#include "mt_defs.h"
#include "WelsThreadLib.h"

//end
#endif
#ifdef USE_X264_ENC
//x264
#include "x264.h"
//end
#endif
#include "../net/protocol/rtp_packet.h"
#include "../net/protocol/core/hg_queue.h"


class VideoEncoder {
public:
#ifdef USE_X264_ENC
    //x264
    x264_param_t x264Param;
    x264_t *h264;
    x264_nal_t *h264nal;
    int h264i_nal;
    int h264i_frame_size;
    //end
#endif
    myQueue *quEncoder;
    double mFrameCounter;
    double mFPS;
    sem_t semEncoder;
    uint32_t input_size;

    int libs=1;//1=openh264,2=x264
#ifdef USE_OPENH264_ENC
    ISVCEncoder *pSVCEncoder ;
    SSourcePicture *pSrcPic;
    SEncParamExt sSvcParam;
    SFrameBSInfo sFbi;
#endif
#ifdef USE_HW_ENC
    AMediaCodec *mEncoder;
    AMediaMuxer *mMuxer;
    AMediaCodecBufferInfo *mBufferInfo;
    bool mMuxerStarted;
    int mTrackIndex;
#endif
    pthread_mutex_t mtxEncoder;

    bool finished;
    bool sourceFin;

    int64_t iFrameIdx;
    pthread_t pthEncoder;

    int64_t iTotal;
    int32_t iActualFrameEncodedCount = 0;
    VideoEncoder();
    void PushDataIn(uint8_t *data,uint32_t size);
    int InitEncoder();
    int StartRun();
    int DoEncode(uint8_t *yuvData, int width,int height,uint16_t times);
    void DestroySVCEncHandle (void* pEncoder);
#ifdef USE_HW_ENC
    void PrepareEncoder(int width, int height, int fps, std::string strPath);
    bool Write(uint8_t *data,int size, const long long timestamp);
    void End();
    void DrainEncoder(bool endOfStream);
    void ReleaseEncoder();
#endif

#ifdef USE_OPENH264_ENC
    int FillSpecificParameters (SEncParamExt& sParam,int iPicWidth,int iPicHeight);
#endif
    long long ComputePresentationTimeNsec();

    static void *HandleEncoder(void *ctx);
    static void AndroidLog(void* ctx, int level, const char* string);
    static void X264LogHan( void *p_unused, int i_level, const char *psz_fmt, va_list arg );
};


#endif //ANDROID_PROJECT_VIDEOENCODER_H
