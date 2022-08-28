//
// Created by ajl <429200247@qq.com> on 2020/12/20.
//

#include "audio_handle.h"
#include "../net/protocol/core/tools.h"
#include "../net/hg_client.h"
#include "app.h"
#include "handle_com.h"
#include "../net/protocol/rtp_packet.h"

AudioHandle::AudioHandle(audio_config_t *recconfig, audio_config_t *playconfig) {
    aEncoder = new AudioEncoder(recconfig);
    aDecoder = new AudioDecoder(playconfig);

}


int AudioHandle::RunAudioEncoder() {

    if (aEncoder == nullptr || !aEncoder->finished) {
        return (1);
    }
    return 0;
}


int AudioHandle::RunAudioDecoder() {

    if (aDecoder->finished) {
        return 0;
    } else {
        return -1;
    }
}

void AudioHandle::SendData(uint8_t *output, uint32_t size, uint16_t time, int comptype) {
    char contents[512] = {0};
    sprintf(contents, "yyyyyyyyyyyyyyyy 1 1 1yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n");

    // FILE *f2 = fopen(APP_ROOT_"send-.data", "a+");
    // fwrite(contents, 1, strlen(contents), f2);
    // fwrite(output, 1, size, f2);
    // fclose(f2);


    hg_chain_t hct;

    handle.Channel.Drive(nullptr, 0);
    int mfcsize = HgNetabstract::CreateFrameStr(handle.fragCache, &hct, hgapp->ssrc,
                                                (int) ENU_PLTYPEAUDIO, 0, (int) CMD_DEFAULT, time,
                                                (uint8_t *) output, size, HANDLE_MAX_FRAME_SLICE,
                                                comptype);
    //memcpy((char *) hbgt->data + RTP_PACKET_HEAD_SIZE, (uint8_t *)output, size);
    media_frame_chain mfc;

    HandleCom::framebufSet(&mfc,time, &hct, mfcsize, &handle);
    HgSessionC *sesstm = nullptr;
    if (hgapp->huageConn != nullptr) {
        sesstm = hgapp->huageConn->data;
    }
    if (sesstm == nullptr) {
        return;
    }
    //ajltest
    //发现有走网络偶尔呜呜的声音，猜测是时间戳的问题，明天开始给加时间戳，主要验证音频都有哪些问题，然后才是音频多路拼接排除，并且和视频对齐的问题
    //RecvCallback(this,&mfc,sesstm);
    //end

    HgNetabstract::PreSendData(hgapp->hgcc->hnab, &mfc, hgapp->ssrc, (int) ENU_PLTYPEAUDIO,
                               0, sesstm, false, nullptr, comptype);
}

void AudioHandle::RecvCallback(void *ctx, media_frame_chain *mfc, HgSessionC *sess) {
    AudioHandle *ahandle = (AudioHandle *) ctx;
    uint8_t pcmBuf[
            ahandle->aDecoder->renderconfig.channels * ahandle->aDecoder->renderconfig.frameperbuf *
            2];

    if (ahandle->aDecoder == nullptr) {
        return;
    }
/*
char contents[1000];

    FILE *f2 = fopen(APP_ROOT_"recvbak.data", "a+");

    hg_chain_node *tmp11=mfc->hct.left;

    hg_Buf_Gen_t *bttmp= (hg_Buf_Gen_t *)tmp11->data;
    int seq = pktSeq((unsigned char *)bttmp->data, 13);
    int cmd = pktCmd((unsigned char *)bttmp->data, 13);
    int flag=ADEC_HEADER_FLAG(*((unsigned char *)bttmp->data + 1));
    sprintf(contents, "yyyyyyyyyyyyyyyy %d %d %d %dyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n",flag,cmd, seq,bttmp->len);
    fwrite(contents, 1, strlen(contents), f2);

    while(tmp11!=nullptr){
        hg_Buf_Gen_t *bttmp= (hg_Buf_Gen_t *)tmp11->data;

       // int seq = pktSeq((unsigned char *)bttmp->data, 13);
       // int cmd = pktCmd((unsigned char *)bttmp->data, 13);
       // int flag=ADEC_HEADER_FLAG(*((unsigned char *)bttmp->data + 1));
       // sprintf(contents, "yyyyyyyyyyyyyyyy %d %d %d %dyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n",flag,cmd, seq,bttmp->len);

        fwrite((uint8_t *)bttmp->data+bttmp->start, 1, bttmp->len, f2);
        tmp11=tmp11->next;
    }
    fclose(f2);

*/

    hg_chain_node *hcn = mfc->hct.left;
    hg_chain_node *hcnR = mfc->hct.right;
    hg_Buf_Gen_t *hbgtR = (hg_Buf_Gen_t *) hcnR->data;
    param_handle freeHandle = hbgtR->sfree.freehandle;
    hg_Buf_Gen_t *hbgtleft = (hg_Buf_Gen_t *) hcn->data;
    int comptype = pktComp((uint8_t *) hbgtleft->data, 13);




    //ahandle->aDecoder->DoDecode(mfc->hct.left, mfc->size, &pcmBuf, &pcmsize,aframeinfo,sesscpts);
    if (comptype == 0) {
        if (mfc->size - RTP_PACKET_HEAD_SIZE != sizeof(pcmBuf)) {
            freeHandle(hbgtR->sfree.ctx, hbgtR->sfree.params);
            assert(false);
        }

    } else {
        if (mfc->size - RTP_PACKET_HEAD_SIZE != SINGLE_MSG0_LEN + sizeof(MsgObjHead)) {
            freeHandle(hbgtR->sfree.ctx, hbgtR->sfree.params);
            assert(false);
        }
/////////////
        MsgObj0 msgarr;
        hg_chain_node *hcntmp = mfc->hct.left;
        hg_Buf_Gen_t *hbgttmp = nullptr;
        uint8_t vptsmsgca[512];
        MsgObj0 dist[MSG0_MAX_DE];
        int offset = 0;
        while (hcntmp != nullptr) {
            hbgttmp = (hg_Buf_Gen_t *) hcntmp->data;
            int datamove = 0;
            memcpy(vptsmsgca + offset, (char *) hbgttmp->data + hbgttmp->start, hbgttmp->len);
            hcntmp = hcntmp->next;
            offset += hbgttmp->len;
        }

        int dsize = MsgObj0::getdelen(vptsmsgca, offset);
        MsgObj0::decode(vptsmsgca, offset, dist, dsize);
      //  ALOGI(0, "gggg333ggggggggggggggggg 0000 \n");
        for (int i = 0; i < dsize; i++) {
            MsgObj0 tmp = dist[i];
          //  ALOGI(0, "gggg333ggggggggggggggggg ssrc %d vpts %d apts %d\n", tmp.ssrc, tmp.vpts,
           //       tmp.apts);
        }
       // ALOGI(0, "gggg333ggggggggggggggggg 1111 \n");
        uint16_t apts = msgarr.apts;
        uint16_t vpts = msgarr.vpts;

//////////////
        freeHandle(hbgtR->sfree.ctx, hbgtR->sfree.params);
        return;
    }

    /////////////
    hg_Buf_Gen_t *hbgt_ca = nullptr;
    void *data_ca = nullptr;
    int pcmsize = 0;
    hg_chain_node *hcn_ca = mfc->hct.left;
    ALOGI(0, "ggggggggggggggggggggg ld");
    while (hcn_ca != nullptr) {
        hbgt_ca = (hg_Buf_Gen_t *) hcn_ca->data;
        data_ca = (char *) hbgt_ca->data + hbgt_ca->start;
        ALOGI(0, "ggggggggggggggggggggg %ld,%ld,%d,%d,%d seq %d", (int64_t) pcmBuf,
              (int64_t) data_ca, pcmsize, hbgt_ca->len, mfc->size,
              pktTimestamp((uint8_t *) hbgt_ca->data, 20));
        memcpy(pcmBuf + pcmsize, data_ca, hbgt_ca->len);
        pcmsize += hbgt_ca->len;
        hcn_ca = hcn_ca->next;
    }
    //////////////
    /*
    char aaa[200]={0};
    sprintf(aaa,"%s","yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy");
    FILE *f3 = fopen("/data/data/org.libhuagertp.app/recv0-.data", "a+");
    fwrite(aaa, 1, strlen(aaa), f3);
    fwrite(pcmBuf, 1, pcmsize, f3);
    fclose(f3);*/

    App::onRecvAudData_Callback(ISAUDIO, pcmBuf, pcmsize,0);


    freeHandle(hbgtR->sfree.ctx, hbgtR->sfree.params);
}


void AudioHandle::Start() {
    RunAudioDecoder();
    RunAudioEncoder();
}

AudioHandle::~AudioHandle() {
}