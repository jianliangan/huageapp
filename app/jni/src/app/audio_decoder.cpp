//
// Created by ajl on 2021/1/20.
//

#include "audio_decoder.h"
#include "app.h"

AudioDecoder::AudioDecoder(audio_config_t *playconfig) {

    output_size=0;
    decode_buf= nullptr;

    uint32_t err;
    err = 0;
    finished = false;

    renderconfig.channels = playconfig->channels;
    renderconfig.samplesrate = playconfig->samplesrate;
    renderconfig.deep = playconfig->deep;
    renderconfig.frameperbuf = playconfig->frameperbuf;

    InitDecoder();
    if (err != 0) {
        ALOGI(0, "audio Decoder init err");
        return;
    }
    finished = true;
}

void AudioDecoder::EndDecoder() {
#ifdef USEAACAUDIO
    aacDecoder_Close(handle);
#endif
}

void AudioDecoder::InitDecoder() {


    uint8_t *output_buf= nullptr;

#ifdef USEAACAUDIO
    handle = aacDecoder_Open(TT_MP4_ADTS, 1);

    output_size = 2 * sizeof(INT_PCM) * 1024;//这是按2个声道算的
#endif
//((CONF_FRAMEPERBUF*CONF_BITDEPTH+7)>>3)*CONF_CHANNELS
    output_size = renderconfig.frameperbuf*renderconfig.deep*renderconfig.channels>>3;

    decode_buf = new uint8_t[output_size];//按固定一个采样16位计算的


}

int
AudioDecoder::AacDecode(hg_chain_node *hcn, uint32_t size, uint8_t **dst, uint32_t *dstsize) {

    uint8_t *ptr = (uint8_t *) hcn;
    uint8_t *packet = ptr;
    uint32_t n=0, i=0;
    uint32_t valid=0, packet_size=0;
    int frame_size = 0;
#ifdef USEAACAUDIO
    AAC_DECODER_ERROR err;

    if (size < 7)
        return -1;
    if (packet[0] != 0xff || (packet[1] & 0xf0) != 0xf0) {
        ALOGI(0, "Not an ADTS packet ");
        return -1;
    }

//110000 0000 000 长度是13位存储
    uint32_t packet_size_0 = ((packet[3] & 0x03) << 11) | (packet[4] << 3) | (packet[5] >> 5);
    if (packet_size_0 != size) {
        return -1;
    }
    packet_size = size;
    valid = packet_size;

    err = aacDecoder_Fill(handle, &ptr, &packet_size, &valid);
    if (err != AAC_DEC_OK) {
        ALOGI(0, "Fill failed: %x\n", err);
        return -1;
    }

    err = aacDecoder_DecodeFrame(handle, (INT_PCM *) decode_buf, output_size / sizeof(INT_PCM), 0);
    if (err == AAC_DEC_NOT_ENOUGH_BITS) {
        ALOGI(0, "Fill AAC_DEC_NOT_ENOUGH_BITS");
        return -1;
    }
    if (err != AAC_DEC_OK) {
        ALOGI(0, "Decode failed: %x\n", err);
        return -1;
    }

    if (frame_size == 0) {
        CStreamInfo *info = aacDecoder_GetStreamInfo(handle);
        if (!info || info->sampleRate <= 0) {
            ALOGI(0, "No stream info\n");
            return -1;
        }
        frame_size = info->frameSize * info->numChannels;
    }
    *dst = decode_buf;
    *dstsize = sizeof(INT_PCM) * frame_size;
#endif
    return 0;
}

int
AudioDecoder::G711Decode(hg_chain_node *hcn, uint32_t size, uint8_t **dst, uint32_t *dstsize) {

    int16_t *tmp = (int16_t * )(decode_buf);
    //for (int i = 0; i < size; i++) {
    ////

    hg_Buf_Gen_t *hbgt= nullptr;


    void *data= nullptr;
    int sizetmp = 0;
    int sizeall = 0;

    while (hcn != nullptr) {

        hbgt = (hg_Buf_Gen_t *)hcn->data;
        hcn = hcn->next;
        data = (char *)hbgt->data+hbgt->start;
        sizetmp = hbgt->len;
        for (int i = 0; i < sizetmp; i++) {

            *(tmp + sizeall) = MuLaw_Decode(*((int8_t *) data + i));
            sizeall+=1;
        }
    }
    ////

    //}
    *dstsize = (size - RTP_PACKET_HEAD_SIZE) * (renderconfig.deep >> 3);
    *dst = (uint8_t *) tmp;
    return 0;
}

int
AudioDecoder::DoDecode(hg_chain_node *hcn, uint32_t size, uint8_t **dst, uint32_t *dstsize) {
    if (true) {
        G711Decode(hcn, size, dst, dstsize);
    } else {//aac
        AacDecode(hcn, size, dst, dstsize);
    }
    return 0;
}