//
// Created by ajl on 2021/1/17.
//
#if defined(USE_HW_ENC)
#include "media/NdkMediaCrypto.h"
#include "media/NdkMediaCodec.h"
#include "media/NdkMediaError.h"
#include "media/NdkMediaFormat.h"
#include "media/NdkMediaMuxer.h"
#include "media/NdkMediaExtractor.h"
#endif


#include "video_encoder.h"
#include "app.h"


VideoEncoder::VideoEncoder() {
    uint32_t err;
    finished = false;
#if defined(USE_OPENH264_ENC)
    pSVCEncoder = nullptr;
#endif
}

int VideoEncoder::StartRun() {
    int err;
    ALOGI(0, "startRun");
    err = pthread_create(&pthEncoder, NULL, HandleEncoder, this);
    if (err != 0) {
        ALOGI(0, "video encoder thread err");
        return 1;
    }

    return 0;
}

void VideoEncoder::PushDataIn(uint8_t *data, uint32_t size) {

    if (!finished) {
        InitEncoder();
        StartRun();
        return;
    }

    hg_pthread_lock(&mtxEncoder);
    uint32_t ret;
    myQuNode *mqNode = myQuMalloc(quEncoder, &ret);
    if (ret != 0) {
        hg_pthread_unlock(&mtxEncoder);
        if (ret == 1)
            ALOGI(0, "videoEncoder alloc error data size too large");
        else
            ALOGI(0, "videoEncoder alloc error ");
        return;
    }
    uint8_t *recvLine = (uint8_t * )((uint8_t *) mqNode + sizeof(myQuNode));
    mqNode->s = size;
    memcpy(recvLine, data, size);

    myQuRightPush(quEncoder, mqNode);
    hg_pthread_unlock(&mtxEncoder);

    sem_post(&semEncoder);

}

void *VideoEncoder::HandleEncoder(void *ctx) {
    ALOGI(0, "handleEncoder");
    VideoEncoder *vEncoder = (VideoEncoder *) ctx;
    myQuNode *node= nullptr;
    uint8_t *data = new uint8_t[vEncoder->input_size];


    uint8_t *recvLine;
    uint32_t recvLength;
    uint16_t times;

    while (true) {
        hgSemWait(&vEncoder->semEncoder);
        recvLine = nullptr;
        times = 0;
        pthread_mutex_lock(&vEncoder->mtxEncoder);
        node = myQuLeftPopNode(vEncoder->quEncoder);
        if (node != nullptr) {
            recvLine = data;
            recvLength = node->s - 4;
            times = *((uint16_t *) node->d);
            memcpy(data, (uint8_t *) node->d + 4, node->s - 4);
            myQuLeftPopEndFree(vEncoder->quEncoder, node);
        }

        pthread_mutex_unlock(&vEncoder->mtxEncoder);
        if (recvLine == nullptr) {
            continue;
        }


        if (recvLength > RTP_PACKET_HEAD_SIZE) {

            int64_t end0 = hgetSysTimeMicros() / 1000;

            vEncoder->DoEncode(recvLine, hgapp->vHandle->vSource->vWidth,
                               hgapp->vHandle->vSource->vHeight, times);

            int64_t diff = hgetSysTimeMicros() / 1000 - end0;

        }

    }
}

void VideoEncoder::DestroySVCEncHandle(void *pEncoder) {
#if defined(USE_OPENH264_ENC)
    ISVCEncoder *pe=(ISVCEncoder *)pEncoder;
    if (pe) {
        WelsDestroySVCEncoder(pe);
    }
#elif defined(USE_X264_ENC)
    x264_encoder_close( h264 );

#endif
}

void VideoEncoder::AndroidLog(void *ctx, int level, const char *s) {
    //ctx use ENCODER_OPTION_TRACE_CALLBACK_CONTEXT set
    ALOGI(0, "androidLog()  %s\n", s);
}

int VideoEncoder::InitEncoder() {
    int picW, picH, iRet;
#if defined(USE_OPENH264_ENC)

    static int g_LevelSetting = WELS_LOG_ERROR;//WELS_LOG_ERROR  WELS_LOG_DEBUG
    int kiPicResSize;
    int64_t iStart = 0, iTotal = 0;
    iFrameIdx = 0;
    iTotal = 0;
    uint32_t iSourceWidth, iSourceHeight;
    WelsTraceCallback alog = AndroidLog;
    iRet = WelsCreateSVCEncoder(&pSVCEncoder);
    if (iRet) {
        ALOGI(0, "WelsCreateSVCEncoder() failed!!\n");
        goto RETURN;
    }
    memset(&sFbi, 0, sizeof(SFrameBSInfo));
    pSVCEncoder->GetDefaultParams(&sSvcParam);
    pSrcPic = new SSourcePicture;
    if (pSrcPic == NULL) {
        DestroySVCEncHandle(pSVCEncoder);
        iRet = 1;
        goto RETURN;
    }
    //fill default pSrcPic
    pSrcPic->iColorFormat = videoFormatI420;
    pSrcPic->uiTimeStamp = 0;

    pSVCEncoder->SetOption(ENCODER_OPTION_TRACE_LEVEL, &g_LevelSetting);
    pSVCEncoder->SetOption(ENCODER_OPTION_TRACE_CALLBACK, &alog);
    //finish reading the configurations
    pSrcPic->iPicWidth = hgapp->vHandle->vSource->vWidth;
    pSrcPic->iPicHeight = hgapp->vHandle->vSource->vHeight;
    iSourceWidth = pSrcPic->iPicWidth;
    iSourceHeight = pSrcPic->iPicHeight;
    kiPicResSize = iSourceWidth * iSourceHeight * 3 >> 1;

    FillSpecificParameters(sSvcParam, iSourceWidth, iSourceHeight);

    //update pSrcPic
    pSrcPic->iStride[0] = iSourceWidth;
    pSrcPic->iStride[1] = pSrcPic->iStride[2] = pSrcPic->iStride[0] >> 1;

    pSrcPic->pData[0] = nullptr;
    pSrcPic->pData[1] = nullptr;
    pSrcPic->pData[2] = nullptr;//pSrcPic->pData[1] + (iSourceWidth * iSourceHeight >> 2);

    //  sSvcParam.bSimulcastAVC = true;
    if (cmResultSuccess !=
        pSVCEncoder->InitializeExt(&sSvcParam)) { // SVC encoder initialization
        ALOGI(0, "SVC encoder Initialize failed\n");
        DestroySVCEncHandle(pSVCEncoder);
        iRet = 1;
        goto RETURN;
    }
picW=pSrcPic->iPicWidth;
    picH=pSrcPic->iPicHeight;
#elif defined(USE_X264_ENC)
    if( x264_param_default_preset( &x264Param, "medium", NULL ) < 0 ){
        DestroySVCEncHandle(nullptr);
        ALOGI(0, "initEncoder x264 failed");
        return 0;
    };

    //x264Param.i_deepth = 8;
    x264Param.i_csp = X264_CSP_I420;
    x264Param.i_width  = hgapp->vHandle->vSource->vWidth;
    x264Param.i_height = hgapp->vHandle->vSource->vHeight;
    x264Param.b_vfr_input = 0;
    x264Param.b_repeat_headers = 1;
    x264Param.b_annexb = 1;

//x264Param.pf_log = x264LogHan;
//x264Param.p_log_private = NULL;
//x264Param.i_log_level = X264_LOG_DEBUG;
    /* Apply profile restrictions. */
    if( x264_param_apply_profile( &x264Param, "high" ) < 0 ){
        DestroySVCEncHandle(nullptr);
        ALOGI(0, "initEncoder x264_param_apply_profile failed");
        return 0;
    }

    //if( x264_picture_alloc( &h264pic, x264Param.i_csp, x264Param.i_width, x264Param.i_height ) < 0 ){
     //   DestroySVCEncHandle(nullptr);
    //    ALOGI(0, "initEncoder x264_picture_alloc failed");
     //   return 0;
    //}

    h264 = x264_encoder_open( &x264Param );
    if( !h264 ){
        DestroySVCEncHandle(nullptr);
        ALOGI(0, "initEncoder x264_encoder_open failed");
        return 0;
    }
picW=x264Param.i_width;
picH=x264Param.i_height;
#elif defined(USE_HW_ENC)
    PrepareEncoder(hgapp->vHandle->vSource->vWidth, hgapp->vHandleEnc->vSource->vHeight, 25, std::string strPath,"lock.f");
#endif
    /////////////////////////////////////
    input_size =
            picW * picH;//存放时间戳，framelength是采样数，一个采样是16位，2个字节，然后是声道
    input_size = 4 + input_size + (input_size >> 1);//420的尺寸
    //下面是初始化缓冲区
    quEncoder = myQueueCreate(input_size);// 带时间戳
    pthread_mutex_init(&mtxEncoder, NULL);
    sem_init(&semEncoder, 0, 0);

    finished = true;
    iRet = 0;
    ALOGI(0, "initEncoder end");
    RETURN:
    return iRet;
}


int VideoEncoder::DoEncode(uint8_t *yuvData, int width, int height, uint16_t times) {
#if defined(USE_OPENH264_ENC)
    int64_t iStart;

    int iLayerSize = 0;
    int naluStart;
    int iEncFrames;
    // To encoder this frame
    if (pSVCEncoder == nullptr) {
        return 0;
    }
    iStart = hgetSysTimeMicros();
    pSrcPic->pData[0] = (uint8_t *) yuvData;
    pSrcPic->pData[1] = pSrcPic->pData[0] + (width * height);
    pSrcPic->pData[2] = pSrcPic->pData[1] + ((width * height) >> 2);

    pSrcPic->uiTimeStamp = WELS_ROUND(iFrameIdx * (1000 / sSvcParam.fMaxFrameRate));


    //std::string ll="ajl";
    //FILE *f2 = fopen("/data/data/org.libhuagertp.app/0.data", "a");
    //fwrite(ll.c_str(), 1, ll.length(), f2);
    //fwrite(pSrcPic->pData[0], 1, width * height, f2);
    //fclose(f2);


    ///////////////
    //push
    /*
    if (hgapp->vHandle->vManDec->vDecoders[0] == nullptr) {
        hgapp->vHandle->vManDec->vDecoders[0] = new videoDecoder();
    }
    chainPic *chainpps = hgapp->vHandle->vManDec->vDecoders[0]->chainPics;
    hg_pthread_lock(&hgapp->vHandle->vManDec->vDecoders[0]->mtxyuvPic);
    uint8_t *mall = myPoolAlloc(hgapp->vHandle->vManDec->vDecoders[0]->yuvPicPool, chainpps->picSize + sizeof(chainPicNode));
    // uint8_t *mall = (uint8_t *)malloc(chainpps->picSize + sizeof(chainPicNode));
    hg_pthread_unlock(&hgapp->vHandle->vManDec->vDecoders[0]->mtxyuvPic);

    chainPicNode *chainpNode = (chainPicNode *) mall;
    chainpNode->pic = mall + sizeof(chainPicNode);
    memcpy(chainpNode->pic, yuvData, width*height);
    memcpy(chainpNode->pic + width*height, yuvData+(width*height), (width*height) >> 2);
    memcpy(chainpNode->pic + width*height + ((width*height) >> 2), yuvData+(width*height)+((width*height) >> 2),
           (width*height) >> 2);
    chainpNode->next = nullptr;
    chainpNode->width=width;
    chainpNode->height=height;
    hg_pthread_lock(&hgapp->vHandle->vManDec->vDecoders[0]->mtxchainPicsDec);

    if (hgapp->vHandle->vManDec->vDecoders[0]->chainPics[0].right == nullptr) {
        hgapp->vHandle->vManDec->vDecoders[0]->chainPics[0].right = chainpNode;
        hgapp->vHandle->vManDec->vDecoders[0]->chainPics[0].left = chainpNode;
    } else {
        hgapp->vHandle->vManDec->vDecoders[0]->chainPics[0].right->next = chainpNode;
        hgapp->vHandle->vManDec->vDecoders[0]->chainPics[0].right = chainpNode;
    }

    hg_pthread_unlock(&hgapp->vHandle->vManDec->vDecoders[0]->mtxchainPicsDec);
*/
    //end push
    ///////////////


    int64_t end0 = hgetSysTimeMicros() / 1000;

    iEncFrames = pSVCEncoder->EncodeFrame(pSrcPic, &sFbi);
    int64_t diff = hgetSysTimeMicros() / 1000 - end0;


    iTotal += hgetSysTimeMicros() - iStart;
    ++iFrameIdx;
    if (videoFrameTypeSkip == sFbi.eFrameType) {
        goto RETURN;
    }

    if (iEncFrames == cmResultSuccess) {

        int iLayer = 0;


        while (iLayer < sFbi.iLayerNum) {
            SLayerBSInfo *pLayerBsInfo = &sFbi.sLayerInfo[iLayer];
            if (pLayerBsInfo != NULL) {
                naluStart = 0;
                int iNalIdx = 0;
                if (pLayerBsInfo->uiSpatialId != 0) {
                    ++iLayer;
                    continue;
                }

                do {
                    iLayerSize = pLayerBsInfo->pNalLengthInByte[iNalIdx];
                    //  hgapp->vHandle->SendData(pLayerBsInfo->pBsBuf + naluStart, iLayerSize, times);

                    {

                        hgapp->vHandle->InitvDecoders(0);
                        int64_t end0 = hgetSysTimeMicros() / 1000;
hgapp->vHandle->DecoderDecode(0,pLayerBsInfo->pBsBuf + naluStart, iLayerSize);


                        int64_t diff = hgetSysTimeMicros() / 1000 - end0;

                    }

                    naluStart += iLayerSize;
                    iNalIdx++;
                } while (iNalIdx < pLayerBsInfo->iNalCount);
            }
            ++iLayer;
        }
        ++iActualFrameEncodedCount; // excluding skipped frame time
    } else {
        ALOGI(0, "mydoEncode 4");
        ALOGI(0, "EncodeFrame(), ret: %d, frame index: %lld.\n", iEncFrames, iFrameIdx);

    }


    if (iActualFrameEncodedCount > 0) {
        double dElapsed = iTotal / 1e6;
        // ALOGI(0,
        //       "Width:\t\t%d\nHeight:\t\t%d\nFrames:\t\t%d\nencode time:\t%f sec\nFPS:\t\t%f fps\n",
        //       sSvcParam.iPicWidth, sSvcParam.iPicHeight,
        //       iActualFrameEncodedCount, dElapsed, (iActualFrameEncodedCount * 1.0) / dElapsed);
    }

    /////////////////////////////////
    RETURN:
    return 0;
#elif defined(USE_X264_ENC)
    static int i_frame=0;
        x264_picture_t h264pic;
        x264_picture_t h264pic_out;
        x264_picture_init(&h264pic);
        h264pic.i_pts = i_frame;
        h264pic.img.i_csp = x264Param.i_csp;
        h264pic.img.i_plane = 3;

        h264pic.img.i_stride[0]=width;
        h264pic.img.i_stride[1]=width>>1;
        h264pic.img.i_stride[2]=width>>1;

        h264pic.img.plane[0]=(uint8_t *)yuvData;
        h264pic.img.plane[1]=(uint8_t *)yuvData + (width * height);
        h264pic.img.plane[2]=(uint8_t *)yuvData+ (width * height) + (width * height>>2);

        i_frame++;
        int64_t end0 = hgetSysTimeMicros() / 1000;
        h264i_frame_size = x264_encoder_encode( h264, &h264nal, &h264i_nal, &h264pic, &h264pic_out );
        int64_t diff = hgetSysTimeMicros() / 1000 - end0;

        if( h264i_frame_size < 0 )
        {
            ALOGI(0, "encode error ");
        }
        else if( h264i_frame_size )
        {

            //  hgapp->vHandle->SendData(pLayerBsInfo->pBsBuf + naluStart, iLayerSize, times);

            {

                hgapp->vHandle->InitvDecoders(0);
                int64_t end0 = hgetSysTimeMicros() / 1000;
                hgapp->vHandle->DecoderDecode(0,h264nal->p_payload, h264i_frame_size);


                int64_t diff = hgetSysTimeMicros() / 1000 - end0;

            }


        }

#endif
    return 0;
}


#if defined(USE_OPENH264_ENC)
int VideoEncoder::FillSpecificParameters(SEncParamExt &sParam, int iPicWidth, int iPicHeight) {
    /* Test for temporal, spatial, SNR scalability */
    //sParam.iUsageType = CAMERA_VIDEO_REAL_TIME;
    sParam.fMaxFrameRate = 30.0f;                // input frame rate
    sParam.iPicWidth = iPicWidth;                 // width of picture in samples
    sParam.iPicHeight = iPicHeight;                  // height of picture in samples
    sParam.iTargetBitrate = 1500000;              // target bitrate desired
    sParam.iMaxBitrate = UNSPECIFIED_BIT_RATE;
    sParam.iRCMode = RC_QUALITY_MODE;      //  rc mode control
    sParam.iTemporalLayerNum = 1;    // layer number at temporal level
    sParam.iSpatialLayerNum = 1;    // layer number at spatial level
    sParam.bEnableDenoise = 0;    // denoise control
    sParam.bEnableBackgroundDetection = 1; // background detection control
    sParam.bEnableAdaptiveQuant = 1; // adaptive quantization control
    //sParam.bEnableFrameSkip = 1; // frame skipping
    sParam.bEnableLongTermReference = 0; // long term reference control
    sParam.iLtrMarkPeriod = 30;
    //sParam.uiIntraPeriod = 320;           // period of Intra frame
    //sParam.eSpsPpsIdStrategy = INCREASING_ID;
    sParam.bPrefixNalAddingCtrl = 0;
    sParam.iMultipleThreadIdc         = 0;
    sParam.iComplexityMode = HIGH_COMPLEXITY;//LOW_COMPLEXITY;
    //sParam.bSimulcastAVC = false;
    sParam.iEntropyCodingModeFlag     = 0;
    //sParam.bEnableSceneChangeDetect=1;
    int iIndexLayer = 0;
    //sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_BASELINE;
    sParam.sSpatialLayers[iIndexLayer].iVideoWidth = iPicWidth;
    sParam.sSpatialLayers[iIndexLayer].iVideoHeight = iPicHeight;
    sParam.sSpatialLayers[iIndexLayer].fFrameRate = 30;
    sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 1500000;
    sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = 3000000;
    sParam.sSpatialLayers[iIndexLayer].sSliceArgument.uiSliceMode = SM_FIXEDSLCNUM_SLICE;
    sParam.sSpatialLayers[iIndexLayer].sSliceArgument.uiSliceNum = 1;
    //ALOGI(0, "ccccccccccccccccccccccccc --- %d --%d",iPicWidth,iPicHeight);
/*
    ++iIndexLayer;
    sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_SCALABLE_BASELINE;
    sParam.sSpatialLayers[iIndexLayer].iVideoWidth = 320;
    sParam.sSpatialLayers[iIndexLayer].iVideoHeight = 180;
    sParam.sSpatialLayers[iIndexLayer].fFrameRate = 15.0f;
    sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 160000;
    sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;
    sParam.sSpatialLayers[iIndexLayer].sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;

    ++iIndexLayer;
    sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_SCALABLE_BASELINE;
    sParam.sSpatialLayers[iIndexLayer].iVideoWidth = 640;
    sParam.sSpatialLayers[iIndexLayer].iVideoHeight = 360;
    sParam.sSpatialLayers[iIndexLayer].fFrameRate = 30.0f;
    sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 512000;
    sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;
    sParam.sSpatialLayers[iIndexLayer].sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;
    sParam.sSpatialLayers[iIndexLayer].sSliceArgument.uiSliceNum = 1;

    ++iIndexLayer;
    sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_SCALABLE_BASELINE;
    sParam.sSpatialLayers[iIndexLayer].iVideoWidth = 1280;
    sParam.sSpatialLayers[iIndexLayer].iVideoHeight = 720;
    sParam.sSpatialLayers[iIndexLayer].fFrameRate = 30.0f;
    sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 1500000;
    sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;
    sParam.sSpatialLayers[iIndexLayer].sSliceArgument.uiSliceMode = SM_SINGLE_SLICE;
    sParam.sSpatialLayers[iIndexLayer].sSliceArgument.uiSliceNum = 1;
*/
/*
    float fMaxFr = sParam.sSpatialLayers[sParam.iSpatialLayerNum - 1].fFrameRate;
    for (int32_t i = sParam.iSpatialLayerNum - 2; i >= 0; --i) {
        if (sParam.sSpatialLayers[i].fFrameRate > fMaxFr + EPSN)
            fMaxFr = sParam.sSpatialLayers[i].fFrameRate;
    }
    sParam.fMaxFrameRate = fMaxFr;
*/
    return 0;
}
#endif

void VideoEncoder::X264LogHan(void *p_unused, int i_level, const char *psz_fmt, va_list arg) {
#if defined(USE_X264_ENC)
    const char *  psz_prefix="ddddd";
    char out[4096];
    switch( i_level )
    {
        case X264_LOG_ERROR:
            psz_prefix = "error";
            break;
        case X264_LOG_WARNING:
            psz_prefix = "warning";
            break;
        case X264_LOG_INFO:
            psz_prefix = "info";
            break;
        case X264_LOG_DEBUG:
            psz_prefix = "debug";
            break;
        default:
            psz_prefix = "unknown";
            break;
    }
    vsnprintf(out, sizeof(out), psz_fmt, arg);
    ALOGI(0, "x264 %s,,%s\n",psz_prefix, out);
#endif
}

#ifdef USE_HW_ENC
void VideoEncoder::PrepareEncoder(int width, int height, int fps, std::string strPath) {

    //mWidth = width;
    //mHeight = height;
    //mFps = fps;

    AMediaFormat *format = AMediaFormat_new();
    AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, width);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, height);

    AMediaFormat_setInt32(format,AMEDIAFORMAT_KEY_COLOR_FORMAT, COLOR_FORMAT_SURFACE);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_BIT_RATE, 500000);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, 25);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 1);

    const char *s = AMediaFormat_toString(format);
    ALOGI(0, "hw encoder video format: %s", s);


    mEncoder = AMediaCodec_createEncoderByType("video/avc");


    media_status_t status = AMediaCodec_configure(mEncoder, format, NULL, NULL,
                                                  AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
    if (status != 0) {
        ALOGI(0, "AMediaCodec_configure() failed with error %i for format %u");
    } else {

    }
    AMediaFormat_delete(format);

    FILE *fp = fopen(strPath.c_str(), "wb");

    if (fp != NULL) {
        mFd = fileno(fp);
    } else {
        mFd = -1;
        ALOGI(0,  "create file %s fail", strPath.c_str());
    }

    if(mMuxer == NULL)
        mMuxer = AMediaMuxer_new(mFd, AMEDIAMUXER_OUTPUT_FORMAT_MPEG_4);

    mMuxerStarted = false;

    fclose(fp);

}

bool VideoEncoder::Write(uint8_t *data,int size, const long long timestamp){
    // Feed any pending encoder output into the muxer.
    DrainEncoder(false);

    if(size==0) return false;

    // Generate a new frame of input.

    /**
                  * Get the index of the next available input buffer. An app will typically use this with
                  * getInputBuffer() to get a pointer to the buffer, then copy the data to be encoded or decoded
                  * into the buffer before passing it to the codec.
                  */
    ssize_t inBufferIdx = AMediaCodec_dequeueInputBuffer(mEncoder, TIMEOUT_USEC);

    /**
                  * Get an input buffer. The specified buffer index must have been previously obtained from
                  * dequeueInputBuffer, and not yet queued.
                  */
    size_t out_size;
    uint8_t* inBuffer = AMediaCodec_getInputBuffer(mEncoder, inBufferIdx, &out_size);

    memcpy(inBuffer, data, size);

    /**
          * Send the specified buffer to the codec for processing.
          */
    //int64_t presentationTimeNs = timestamp;
    int64_t presentationTimeNs = ComputePresentationTimeNsec();

    media_status_t status = AMediaCodec_queueInputBuffer(mEncoder, inBufferIdx, 0, out_size, presentationTimeNs, size==0 ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);

    if(status == AMEDIA_OK){
        //qDebug() << "Successfully pushed frame to input buffer";
    }
    else{
        ALOGI(0, "Something went wrong while pushing frame to input buffer");
        return false;
    }

    // Submit it to the encoder.  The eglSwapBuffers call will block if the input
    // is full, which would be bad if it stayed full until we dequeued an output
    // buffer (which we can't do, since we're stuck here).  So long as we fully drain
    // the encoder before supplying additional input, the system guarantees that we
    // can supply another frame without blocking.
    //qDebug() << "sending frame " << i << " to encoder";
    //AMediaCodec_flush(mEncoder);
    return true;
}


void VideoEncoder::End(){
    ALOGI(0, "End of recording called!");
    // Send the termination frame
    ssize_t inBufferIdx = AMediaCodec_dequeueInputBuffer(mEncoder, TIMEOUT_USEC);
    size_t out_size;
    uint8_t* inBuffer = AMediaCodec_getInputBuffer(mEncoder, inBufferIdx, &out_size);
    int64_t presentationTimeNs = ComputePresentationTimeNsec();
    ALOGI(0,  "Sending EOS");
    media_status_t status = AMediaCodec_queueInputBuffer(mEncoder, inBufferIdx, 0, out_size, presentationTimeNs, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
    // send end-of-stream to encoder, and drain remaining output

    DrainEncoder(true);

    ReleaseEncoder();

    // To test the result, open the file with MediaExtractor, and get the format.  Pass
    // that into the MediaCodec decoder configuration, along with a SurfaceTexture surface,
    // and examine the output with glReadPixels.
}


/**
     * Extracts all pending data from the encoder.
     * <p>
     * If endOfStream is not set, this returns when there is no more data to drain.  If it
     * is set, we send EOS to the encoder, and then iterate until we see EOS on the output.
     * Calling this with endOfStream set should be done once, right before stopping the muxer.
     */
void VideoEncoder::DrainEncoder(bool endOfStream) {

    if (endOfStream) {
        ALOGI(0, "Draining encoder to EOS");
        // only API >= 26
        // Send an empty frame with the end-of-stream flag set.
        // AMediaCodec_signalEndOfInputStream();
        // Instead, we construct that frame manually.
    }




    while (true) {
        ssize_t encoderStatus = AMediaCodec_dequeueOutputBuffer(mEncoder, &mBufferInfo, TIMEOUT_USEC);


        if (encoderStatus == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            // no output available yet
            if (!endOfStream) {
                return;
                //break;      // out of while
            }
            if(endOfStream){
                ALOGI(0, "no output available, spinning to await EOS");
                return;
            }

        } else if (encoderStatus == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
            // not expected for an encoder
        } else if (encoderStatus == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            // should happen before receiving buffers, and should only happen once
            if (mMuxerStarted) {
                ALOGI(0,  "ERROR: format changed twice");
            }
            AMediaFormat* newFormat = AMediaCodec_getOutputFormat(mEncoder);

            if(newFormat == nullptr){
                ALOGI(0, "Unable to set new format.");
            }

            ALOGI(0, "encoder output format changed: " + QString::fromStdString(AMediaFormat_toString(newFormat)));

            // now that we have the Magic Goodies, start the muxer
            mTrackIndex = AMediaMuxer_addTrack(mMuxer, newFormat);
            media_status_t err = AMediaMuxer_start(mMuxer);

            if(err != AMEDIA_OK){
                ALOGI(0,  "Error occurred: ");
            }

            mMuxerStarted = true;
        } else if (encoderStatus < 0) {
            ALOGI(0,  "unexpected result from encoder.dequeueOutputBuffer: %d " + encoderStatus);
            // let's ignore it
        } else {

            size_t out_size;
            uint8_t* encodedData = AMediaCodec_getOutputBuffer(mEncoder, encoderStatus, &out_size);

            if(out_size <= 0){
                ALOGI(0, "Encoded data of size 0.");
            }

            if (encodedData == nullptr) {
                ALOGI(0,  "encoderOutputBuffer %d was null",encoderStatus);
            }


            if ((mBufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG) != 0) {
                // The codec config data was pulled out and fed to the muxer when we got
                // the INFO_OUTPUT_FORMAT_CHANGED status.  Ignore it.
                ALOGI(0,  "ignoring BUFFER_FLAG_CODEC_CONFIG");
                mBufferInfo.size = 0;
            }

            if (mBufferInfo.size != 0) {
                if (!mMuxerStarted) {
                    ALOGI(0,   "muxer hasn't started");
                }


                // adjust the ByteBuffer values to match BufferInfo (not needed?)
                //encodedData.position(mBufferInfo.offset);
                //encodedData.limit(mBufferInfo.offset + mBufferInfo.size);

                AMediaMuxer_writeSampleData(mMuxer, mTrackIndex, encodedData, &mBufferInfo);
                //qDebug() << "sent " + QString::number(mBufferInfo.size) + " bytes to muxer";
            }
            else{
                ALOGI(0,   "mBufferInfo empty %d" ,mBufferInfo.size);
            }

            AMediaCodec_releaseOutputBuffer(mEncoder, encoderStatus, false);

            if ((mBufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) != 0) {
                if (!endOfStream) {
                    ALOGI(0,  "reached end of stream unexpectedly");
                } else {
                    ALOGI(0,   "end of stream reached");

                }
                break;      // out of while
            }
        }
    }
}

/**
 * Releases encoder resources.  May be called after partial / failed initialization.
 */
void VideoEncoder::ReleaseEncoder() {
    ALOGI(0,  "releasing encoder objects");
    if (mEncoder != nullptr) {
        AMediaCodec_stop(mEncoder);
    }

    if (mMuxer != nullptr) {
        AMediaMuxer_stop(mMuxer);
    }

    if (mEncoder != nullptr) {
        AMediaCodec_delete(mEncoder);
        mEncoder = nullptr;
    }

    if (mMuxer != nullptr) {
        AMediaMuxer_delete(mMuxer);
        mMuxer = nullptr;
    }

    isRunning = false;
   // emit recordingFinished();
}
#endif

/**
         * Generates the presentation time for frame N, in nanoseconds.
         */

long long VideoEncoder::ComputePresentationTimeNsec() {
    mFrameCounter++;
    double timePerFrame = 1000000.0 / mFPS;
    return static_cast<long long>(mFrameCounter * timePerFrame);
}
