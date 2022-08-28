//
// Created by ajl on 2021/1/20.
//

#include "video_decoder.h"
#include "app.h"

VideoDecoder::VideoDecoder() {

    finished = false;
    InitDecoder();

    pthread_mutex_init(&mtxyuvPic, NULL);
    pthread_mutex_init(&mtxchainPicsDec, NULL);

    chainPics[0].picSize = 0;
    chainPics[1].picSize = 0;
    chainPics[2].picSize = 0;

    finished = true;
}

void VideoDecoder::EndDecoder() {
#if defined(USE_OPENH264_DEC)

    if (pDecoder) {
        pDecoder->Uninitialize();

        WelsDestroyDecoder(pDecoder);
    }
#elif defined(USE_FF_DEC)
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
#endif

}

void VideoDecoder::InitDecoder() {

#if defined(USE_OPENH264_DEC)
    uiTimeStamp = 0;
    static int g_LevelSetting = WELS_LOG_ERROR;//WELS_LOG_ERROR  WELS_LOG_DEBUG
    WelsTraceCallback alog = AndroidLog;

    sDecParam.sVideoProperty.size = sizeof(sDecParam.sVideoProperty);
    sDecParam.eEcActiveIdc = ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE;
    sDecParam.uiTargetDqLayer = (uint8_t) - 1;
    //sDecParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
    sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_SVC;
    sDecParam.bParseOnly = false;
    if (WelsCreateDecoder(&pDecoder) || (NULL == pDecoder)) {
        ALOGI(0, "Create Video Decoder failed.");
        return;
    }
    int32_t iThreadCount = 0;
    pDecoder->SetOption(DECODER_OPTION_NUM_OF_THREADS, &iThreadCount);
    pDecoder->SetOption(DECODER_OPTION_TRACE_LEVEL, &g_LevelSetting);
    pDecoder->SetOption(DECODER_OPTION_TRACE_CALLBACK, &alog);
    if (pDecoder->Initialize(&sDecParam)) {
        ALOGI(0, "Decoder initialization failed.");
        return;
    }
    pDecoder->SetOption(DECODER_OPTION_ERROR_CON_IDC, &sDecParam.eEcActiveIdc);
#elif defined(USE_FF_DEC)
    AVCodecID codec_id = AV_CODEC_ID_H264;

    AVCodec *pCodec=nullptr;





    pCodec = avcodec_find_decoder(codec_id);
    if (!pCodec) {
        ALOGI(0, "Codec not found\n");
        return;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx){
        ALOGI(0, "Could not allocate video codec context\n");
        return;
    }
    //if (pCodec->capabilities&AV_CODEC_CAP_TRUNCATED)
   //     pCodecCtx->flags |= AVCODEC_FLAG_TRUNCATED; /* we do not send complete frames */

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        ALOGI(0, "Could not open codec\n");
        return ;
    }
#if defined(USE_FF_DEC_GF2)
        pCodecCtx->get_buffer2 = Get_buffer;
#endif

    pCodecCtx->opaque=this;
    pFrame = av_frame_alloc();
    av_init_packet(&packet);
#endif

}

void VideoDecoder::DoDecode(uint8_t *naluData, uint32_t size) {
#if defined(USE_OPENH264_DEC)
    uiTimeStamp++;
    int iRet = dsErrorFree;

    uint8_t *mall=nullptr;
    memset(&sDstBufInfo, 0, sizeof(SBufferInfo));
    sDstBufInfo.uiInBsTimeStamp = uiTimeStamp;


    /*
    std::string ll = "ajl";
    FILE *f2 = fopen("/data/data/org.libhuagertp.app/1.data", "a");
    fwrite(ll.c_str(), 1, ll.length(), f2);
    fwrite(naluData, 1, size, f2);
    fclose(f2);*/

    pData[0] = NULL;
    pData[1] = NULL;
    pData[2] = NULL;

    int64_t end0 = hgetSysTimeMicros() / 1000;
    iRet = pDecoder->DecodeFrameNoDelay(naluData, size, pData, &sDstBufInfo);
    //iRet=pDecoder->DecodeFrame2(naluData, size, pData, &sDstBufInfo);

    int64_t diff = hgetSysTimeMicros() / 1000 - end0;


    if (sDstBufInfo.iBufferStatus == 1) {

        int width = sDstBufInfo.UsrData.sSystemBuffer.iWidth;
        int height = sDstBufInfo.UsrData.sSystemBuffer.iHeight;
        int ystride = sDstBufInfo.UsrData.sSystemBuffer.iStride[0];
        int uvstride = sDstBufInfo.UsrData.sSystemBuffer.iStride[1];
        uint8_t *ydata = sDstBufInfo.pDst[0];
        uint8_t *udata = sDstBufInfo.pDst[1];
        uint8_t *vdata = sDstBufInfo.pDst[2];


        //std::string ll="ajl";
        //FILE *f2 = fopen("/data/data/org.libhuagertp.app/1.data", "a");

        // fwrite(ll.c_str(), 1, ll.length(), f2);
        //fwrite(ydata, 1, width * height, f2);
        // fclose(f2);

        if (chainPics[0].picSize == 0) {
            chainPic chainp;
            chainp.right = nullptr;
            chainp.left = nullptr;
            chainp.width = width;
            chainp.height = height;
            chainp.ystride = ystride;
            chainp.uvstride = uvstride;

            chainp.ySize = ystride * height;
            chainp.picSize = chainp.ySize + (chainp.ySize >> 1);

            chainPics[0] = chainp;

            yuvPicPool = myPoolCreate(chainp.picSize + sizeof(chainPicNode));
        }
//push
        if (yuvPicPool == nullptr)
            return;
        static int ajl = 0;
        ajl++;
        if (ajl == 3) {

        }
        chainPic *chainpps = chainPics;


        hg_pthread_lock(&mtxyuvPic);

        mall = myPoolAlloc(yuvPicPool, chainpps->picSize + sizeof(chainPicNode));
        // uint8_t *mall = (uint8_t *)malloc(chainpps->picSize + sizeof(chainPicNode));
        hg_pthread_unlock(&mtxyuvPic);

        //
        chainPicNode *chainpNode = (chainPicNode *) mall;
        chainpNode->pic = mall + sizeof(chainPicNode);


        end0 = hgetSysTimeMicros() / 1000;
        memcpy(chainpNode->pic, ydata, chainpps->ySize);
        memcpy(chainpNode->pic + chainpps->ySize, udata, chainpps->ySize >> 2);
        memcpy(chainpNode->pic + chainpps->ySize + (chainpps->ySize >> 2), vdata,
               chainpps->ySize >> 2);
        int64_t diff = hgetSysTimeMicros() / 1000 - end0;

        chainpNode->next = nullptr;
        chainpNode->width = width;
        chainpNode->height = height;
        chainpNode->ystride = chainpps->ystride;
        chainpNode->uvstride = chainpps->uvstride;
        hg_pthread_lock(&mtxchainPicsDec);

        if (chainPics[0].right == nullptr) {
            chainPics[0].right = chainpNode;
            chainPics[0].left = chainpNode;
        } else {
            chainPics[0].right->next = chainpNode;
            chainPics[0].right = chainpNode;
        }

        hg_pthread_unlock(&mtxchainPicsDec);
        //end push


    }

    return;
#elif defined(USE_FF_DEC)
#if defined(USE_FF_DEC_GF2)


       uint8_t *mall=nullptr;
       int ret, got_picture;

       packet.data = naluData;     //一帧数据地址
       packet.size = size; //一帧数据长度
       ////ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);




       ret = avcodec_send_packet(pCodecCtx, &packet);
       if (ret == AVERROR(EAGAIN)) // 没有数据送入,但是可以继续可以从内部缓冲区读取编码后的视频包
       {
           ALOGI(0, "<DecodePktToFrame> avcodec_send_frame() EAGAIN\n");
       }
       else if (ret == AVERROR_EOF) // 数据包送入结束不再送入,但是可以继续可以从内部缓冲区读取编码后的视频包
       {
           ALOGI(0, "<DecodePktToFrame> avcodec_send_frame() AVERROR_EOF\n");
       }
       else if (ret < 0)  // 送入输入数据包失败
       {
           ALOGI(0, "<DecodePktToFrame> [ERROR] fail to avcodec_send_frame(), res=%d\n", ret);
           return ;
       }


       // 获取解码后的视频帧或者音频帧

       ret       = avcodec_receive_frame(pCodecCtx, pFrame);
       if (ret == AVERROR(EAGAIN)) // 当前这次没有解码后的音视频帧输出,但是后续可以继续读取
       {
           ALOGI(0,"<CAvVideoDecComp::DecodeVideoPkt> no data output\n");

           return ;
       }
       else if (ret == AVERROR_EOF) // 解码缓冲区已经刷新完成,后续不再有数据输出
       {
           ALOGI(0,"<DecodePktToFrame> avcodec_receive_packet() EOF\n");

           return ;
       }
       else if (ret < 0)
       {
           ALOGI(0,"<DecodePktToFrame> [ERROR] fail to avcodec_receive_packet(), res=%d\n", ret);

           return ;
       }

       if (ret < 0) {
           ALOGI(0, "Decode Error.(解码错误)\n");
           return;
       }
       if (ret>=0) {
           int width = pFrame->width;
           int height = pFrame->height;
           chainPic *chainpps = chainPics;

           static uint64_t tt0=0;


           tt0=hgetSysTimeMicros() / 1000;
//push
           if(yuvPicPool== nullptr)
               return;

           //??????????弄到这了，闪烁
           chainPicNode *chainpNode = (chainPicNode *) (pFrame->data[0]-sizeof(chainPicNode));
           chainpNode->pic = pFrame->data[0];
           chainpNode->next = nullptr;
           chainpNode->width = width;
           chainpNode->height = height;
           chainpNode->ystride = chainpps->ystride;
           chainpNode->uvstride = chainpps->uvstride;
           hg_pthread_lock(&mtxchainPicsDec);

           if (chainPics[0].right == nullptr) {
               chainPics[0].right = chainpNode;
               chainPics[0].left = chainpNode;
           } else {
               chainPics[0].right->next = chainpNode;
               chainPics[0].right = chainpNode;
           }
           hg_pthread_unlock(&mtxchainPicsDec);
           //end push
       }
       return;
#else
       uint8_t *mall=nullptr;
       int ret, got_picture;

       packet.data = naluData;     //一帧数据地址
       packet.size = size; //一帧数据长度

       ret = avcodec_send_packet(pCodecCtx, &packet);
       if (ret == AVERROR(EAGAIN)) // 没有数据送入,但是可以继续可以从内部缓冲区读取编码后的视频包
       {
           ALOGI(0, "<DecodePktToFrame> avcodec_send_frame() EAGAIN\n");
       }
       else if (ret == AVERROR_EOF) // 数据包送入结束不再送入,但是可以继续可以从内部缓冲区读取编码后的视频包
       {
           ALOGI(0, "<DecodePktToFrame> avcodec_send_frame() AVERROR_EOF\n");
       }
       else if (ret < 0)  // 送入输入数据包失败
       {
           ALOGI(0, "<DecodePktToFrame> [ERROR] fail to avcodec_send_frame(), res=%d\n", ret);
           return ;
       }


       // 获取解码后的视频帧或者音频帧

       ret       = avcodec_receive_frame(pCodecCtx, pFrame);
       if (ret == AVERROR(EAGAIN)) // 当前这次没有解码后的音视频帧输出,但是后续可以继续读取
       {
           ALOGI(0,"<CAvVideoDecComp::DecodeVideoPkt> no data output\n");

           return ;
       }
       else if (ret == AVERROR_EOF) // 解码缓冲区已经刷新完成,后续不再有数据输出
       {
           ALOGI(0,"<DecodePktToFrame> avcodec_receive_packet() EOF\n");

           return ;
       }
       else if (ret < 0)
       {
           ALOGI(0,"<DecodePktToFrame> [ERROR] fail to avcodec_receive_packet(), res=%d\n", ret);

           return ;
       }

       if (ret < 0) {
           ALOGI(0, "Decode Error.(解码错误)\n");
           return;
       }
       if (ret>=0) {

           int width = pFrame->width;
           int height = pFrame->height;
           int ystride = pFrame->linesize[0];
           int uvstride = pFrame->linesize[1];
           uint8_t *ydata = pFrame->data[0];
           uint8_t *udata = pFrame->data[1];
           uint8_t *vdata = pFrame->data[2];


           //std::string ll="ajl";
           //FILE *f2 = fopen("/data/data/org.libhuagertp.app/1.data", "a");
           // fwrite(ll.c_str(), 1, ll.length(), f2);
           //fwrite(ydata, 1, width * height, f2);
           // fclose(f2);

           if (chainPics[0].picSize == 0) {
               chainPic chainp;
               chainp.right = nullptr;
               chainp.left = nullptr;
               chainp.width = width;
               chainp.height = height;
               chainp.ystride = ystride;
               chainp.uvstride = uvstride;

               chainp.ySize = ystride * height;
               chainp.picSize = chainp.ySize + (chainp.ySize >> 1);

               chainPics[0] = chainp;

               yuvPicPool = myPoolCreate(chainp.picSize + sizeof(chainPicNode));
           }
//push
           if(yuvPicPool== nullptr)
               return;
           static int ajl = 0;
           ajl++;
           if (ajl == 3) {

           }
           chainPic *chainpps = chainPics;


           hg_pthread_lock(&mtxyuvPic);

           mall = myPoolAlloc(yuvPicPool, chainpps->picSize + sizeof(chainPicNode));
           // uint8_t *mall = (uint8_t *)malloc(chainpps->picSize + sizeof(chainPicNode));
           hg_pthread_unlock(&mtxyuvPic);

           //
           chainPicNode *chainpNode = (chainPicNode *) mall;
           chainpNode->pic = mall + sizeof(chainPicNode);


           //end0=hgetSysTimeMicros()/1000;
           memcpy(chainpNode->pic, ydata, chainpps->ySize);
           memcpy(chainpNode->pic + chainpps->ySize, udata, chainpps->ySize >> 2);
           memcpy(chainpNode->pic + chainpps->ySize + (chainpps->ySize >> 2), vdata,
                  chainpps->ySize >> 2);



           chainpNode->next = nullptr;
           chainpNode->width = width;
           chainpNode->height = height;
           chainpNode->ystride = chainpps->ystride;
           chainpNode->uvstride = chainpps->uvstride;
           hg_pthread_lock(&mtxchainPicsDec);

           if (chainPics[0].right == nullptr) {
               chainPics[0].right = chainpNode;
               chainPics[0].left = chainpNode;
           } else {
               chainPics[0].right->next = chainpNode;
               chainPics[0].right = chainpNode;
           }

           hg_pthread_unlock(&mtxchainPicsDec);
           //end push
       }
#endif
#endif
    return;


}

chainPicNode *VideoDecoder::PopDecode() {
    chainPicNode *chainPnode=nullptr;
    if (finished == false)
        return nullptr;
    hg_pthread_lock(&mtxchainPicsDec);
    chainPnode = chainPics[0].left;
    if (chainPnode != nullptr) {
        chainPics[0].left = chainPnode->next;
        chainPnode->next = nullptr;
        if (chainPics[0].left == nullptr) {
            chainPics[0].right = nullptr;
        }
    }
    hg_pthread_unlock(&mtxchainPicsDec);
    return chainPnode;
}

void VideoDecoder::QuLength(int *re) {
    if (yuvPicPool == nullptr)
        return;
    re[0] = yuvPicPool->size;
    re[1] = yuvPicPool->free->size;
}

void VideoDecoder::FreePic(uint8_t *pic) {
    if (yuvPicPool == nullptr)
        return;
#if defined(USE_FF_DEC)
    if (libs == 2)
        return;
#endif
    hg_pthread_lock(&mtxyuvPic);
    myPoolFree(yuvPicPool, pic);
    hg_pthread_unlock(&mtxyuvPic);
}

void VideoDecoder::AndroidLog(void *ctx, int level, const char *s) {
    //ctx use ENCODER_OPTION_TRACE_CALLBACK_CONTEXT set
    ALOGI(0, "videoDecoder::androidLog()  %s\n", s);
}

void VideoDecoder::Our_buffer_default_free(void *opaque, uint8_t *data) {
    chainPicNode *cpn = (chainPicNode *) opaque;
    VideoDecoder *vd = (VideoDecoder *) cpn->opaque;
    uint64_t tt0 = 0;


    tt0 = hgetSysTimeMicros() / 1000;
    if (--cpn->ref == 0) {
        hg_pthread_lock(&vd->mtxyuvPic);
        myPoolFree(vd->yuvPicPool, (uint8_t *) cpn);
        hg_pthread_unlock(&vd->mtxyuvPic);
    }


    // empty
}

#ifdef USE_FF_DEC
int VideoDecoder::Get_buffer(AVCodecContext *s, AVFrame *pic, int flags){
    VideoDecoder * vdr=(VideoDecoder *)s->opaque;
    chainPic *chainpps = vdr->chainPics;
    uint8_t *mall=nullptr;
    int ySize=0;




    if (chainpps[0].picSize == 0) {
        chainPic chainp;
        chainp.right = nullptr;
        chainp.left = nullptr;
        chainp.width = pic->width;
        chainp.height = pic->height;
        chainp.ystride = 4*((pic->width>>2)+1);
        chainp.uvstride = chainp.ystride>>1;

        chainp.ySize = chainp.ystride * pic->height;
        chainp.picSize = chainp.ySize + (chainp.ySize >> 1);

        chainpps[0] = chainp;

        vdr->yuvPicPool = myPoolCreate(chainp.picSize + sizeof(chainPicNode));
    }

    uint64_t tt0=0;
    tt0=hgetSysTimeMicros() / 1000;

    hg_pthread_lock(&vdr->mtxyuvPic);
    mall = myPoolAlloc(vdr->yuvPicPool, chainpps->picSize + sizeof(chainPicNode));
    // uint8_t *mall = (uint8_t *)malloc(chainpps->picSize + sizeof(chainPicNode));
    hg_pthread_unlock(&vdr->mtxyuvPic);
    pic->opaque=mall;
    mall = mall + sizeof(chainPicNode);
    chainPic *cp=&chainpps[0];
    ySize=cp->ySize;

    chainPicNode * cpn1=(chainPicNode *)pic->opaque;
    cpn1->opaque=vdr;
    cpn1->ref=3;
    pic->data[0] = mall;
    pic->data[1] = mall + ySize;
    pic->data[2] = mall + ySize + (ySize >> 2);
    pic->linesize[0]=cp->ystride;
    pic->linesize[1]=cp->uvstride;
    pic->linesize[2]=cp->uvstride;
    pic->buf[0] = av_buffer_create(pic->data[0], ySize, Our_buffer_default_free, pic->opaque, 0);
    pic->buf[1] = av_buffer_create(pic->data[1], ySize >>2, Our_buffer_default_free, pic->opaque, 0);
    pic->buf[2] = av_buffer_create(pic->data[2], ySize >>2, Our_buffer_default_free, pic->opaque, 0);

    return 0;
}
#endif

/////////////////////////////////////
chainPicNode *VideoManDec::PopData(int index) {
    if (vDecoders[index] == nullptr || vDecoders[index]->finished == false)
        return nullptr;
    return vDecoders[index]->PopDecode();
}

void VideoManDec::FreePic(int index, uint8_t *pic) {
    if (vDecoders[index] == nullptr)
        return;
    vDecoders[index]->FreePic(pic);
}

void VideoManDec::QuLength(int index, int *re) {
    if (vDecoders[index] == nullptr)
        return;
    vDecoders[index]->QuLength(re);
}
