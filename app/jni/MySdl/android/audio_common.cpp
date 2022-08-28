//
// Created by ajl on 2021/6/22.
//

#include "audio_common.h"

void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX *pFormat,
                             SampleFormat *pSampleInfo_) {

    memset(pFormat, 0, sizeof(*pFormat));

    //  pFormat->formatType = SL_DATAFORMAT_PCM;
    // Only support 2 channels
    // For channelMask, refer to wilhelm/src/android/channels.c for details
    // if (pSampleInfo_->channels_ <= 1) {
    pFormat->numChannels = pSampleInfo_->channels;
    if (pFormat->numChannels == 1)
        pFormat->channelMask = SL_SPEAKER_FRONT_LEFT;
    else
        pFormat->channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;

    pFormat->sampleRate = pSampleInfo_->sampleRate_ * 1000;//这里有个坑，叫毫赫兹
    pFormat->endianness = SL_BYTEORDER_LITTLEENDIAN;

    //format_pcm_ex.sampleRate = format.samplesPerSec;
    /*
     * fixup for android extended representations...
     */

    pFormat->representation = (SLuint32)pSampleInfo_->representation_;

    switch (pFormat->representation) {
        case SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT:
            ALOGI(0,"dfdfddfdfdf 88");
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_8;
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_8;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT:
            ALOGI(0,"dfdfddfdfdf 16");
            pFormat->bitsPerSample =
                    SL_PCMSAMPLEFORMAT_FIXED_16;  // supports 16, 24, and 32
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case SL_ANDROID_PCM_REPRESENTATION_FLOAT:
            pFormat->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_32;
            pFormat->containerSize = SL_PCMSAMPLEFORMAT_FIXED_32;
            pFormat->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
            break;
        case 0:
            break;
        default:
            ;
    }
}