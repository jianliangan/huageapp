//
// Created by ajl on 2021/1/20.
//
/**
 * decoder直接演变成路，每一路有2-3个分辨率流，每个ssrc对应一路
 * 每一路的2-3个流开始前必须有长宽
 * decoder 我把他看成是多线程的，多路编解码，目前是单个的未来改成多路
 * 根据尺寸确定需要几路，也就是几个解码线程
 *
 */
#ifndef ANDROID_PROJECT_VIDEODECODE_H
#define ANDROID_PROJECT_VIDEODECODE_H
#include "../net/protocol/core/tools.h"
#ifdef USE_OPENH264_DEC
//openh264
#include "codec_def.h"
#include "codec_app_def.h"
#include "codec_api.h"
#include "read_config.h"
#include "typedefs.h"
#include "measure_time.h"

//end
#endif
#ifdef USE_FF_DEC
//ffmpeg
extern "C"
{
#include "libavcodec/avcodec.h"
}
#endif


//end

#include "../net/protocol/core/hg_pool.h"

#include "av_internal.h"
#define VIDEOMANADECNUM 3
class VideoDecoder;

class VideoManDec{
public:
    int length=VIDEOMANADECNUM;
    VideoDecoder *vDecoders[VIDEOMANADECNUM];
    chainPicNode *PopData(int index);
    void FreePic(int index,uint8_t *pic);
    void QuLength(int index,int *re);
};

class VideoDecoder {
public:
    pthread_mutex_t mtxyuvPic;
    myPool *yuvPicPool= nullptr;
    pthread_mutex_t mtxchainPicsDec;
    chainPic chainPics[3];
#ifdef USE_FF_DEC
    AVCodecContext *pCodecCtx = NULL;
    AVFrame	*pFrame=nullptr;
    AVPacket packet;
#endif

    bool finished=false;
    uint32_t frame_size=0;
    int libs=2;//2=ffmpeg,3=ffmpeg+interal alloc
    uint8_t *pData[3] = {NULL};
    uint8_t *pDst[3] = {NULL};
    unsigned long long uiTimeStamp = 0;
#if defined(USE_OPENH264_DEC)
    ISVCDecoder *pDecoder = NULL;
    SDecodingParam sDecParam = {0};
    SBufferInfo sDstBufInfo;
#endif
    //对端地址，考虑放到别处

    VideoDecoder();
    ~VideoDecoder();
    void InitDecoder();
    void EndDecoder();
    void DoDecode(uint8_t *naluData, uint32_t size);
    chainPicNode *PopDecode();
    void DestroyPic();
    void FreePic(uint8_t *pic);
    void QuLength(int *re);
    static void AndroidLog(void *ctx, int level, const char *s);
#ifdef USE_FF_DEC
    static int Get_buffer(AVCodecContext *s, AVFrame *frame, int flags);
#endif
    static void Our_buffer_default_free(void *opaque, uint8_t *data);
};


#endif //ANDROID_PROJECT_VIDEODECODE_H
