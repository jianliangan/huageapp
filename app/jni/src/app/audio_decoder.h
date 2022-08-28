//
// Created by ajl on 2021/1/20.
//

#ifndef ANDROID_PROJECT_AUDIODECODE_H
#define ANDROID_PROJECT_AUDIODECODE_H
#ifdef USEAACAUDIO
#include "aacdecoder_lib.h"
#endif
#include "../net/protocol/core/tools.h"
#include "../net/protocol/core/hg_buf_comm.h"
#include "../net/protocol/hg_channel_stru.h"
#include "audio_common.h"


class AudioDecoder {
public:
    bool haslog;
    bool finished;
    uint32_t output_size;

    uint8_t *decode_buf;
    audio_config_t renderconfig;


#ifdef USEAACAUDIO
    HANDLE_AACDECODER handle;
#endif
    //对端地址，考虑放到别处
    FILE *f2;
    AudioDecoder(audio_config_t *playconfig);
    ~AudioDecoder();
    void InitDecoder();
    void EndDecoder();
    int AacDecode(hg_chain_node *hcn, uint32_t size,uint8_t **dst,uint32_t *dstsize);
    int G711Decode(hg_chain_node *hcn, uint32_t size,uint8_t **dst,uint32_t *dstsize);
    int DoDecode(hg_chain_node *hcn, uint32_t size,uint8_t **dst,uint32_t *dstsize);
};


#endif //ANDROID_PROJECT_AUDIODECODE_H
