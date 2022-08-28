//
// Created by ajl <429200247@qq.com> on 2020/12/20.
//

#include "video_handle.h"
#include "../net/protocol/core/tools.h"
#include "app.h"
#include "video_decoder.h"
#include "video_encoder.h"
#include "../net/hg_client.h"
#include "handle_com.h"

VideoHandle::VideoHandle() {
    vSource = new VideoSource();
    lasttime=0;
}

VideoHandle::~VideoHandle() {
}

void VideoHandle::RecvCallback(void *ctx, media_frame_chain *mfc,
                               HgSessionC *sess) {
    VideoHandle *context = (VideoHandle *) ctx;


    hg_chain_node *hcntmp = mfc->hct.left;
    hg_Buf_Gen_t *hbgttmp = nullptr;
    int total = 0;



    FILE *f2 = fopen("/data/data/org.libhuagertp.app/recv.data", "a+");
  int vpts=0;
 bool hashead=false;
    while(hcntmp!= nullptr){
        hbgttmp=(hg_Buf_Gen_t *)hcntmp->data;
        uint8_t *dtmp=(uint8_t *)hbgttmp->data;

        char contents[512] = {0};
        int seq = pktSeq(dtmp, 13);
        int cmd = pktCmd(dtmp, 13);
        int time=pktTimestamp(dtmp, 13);
        int flat=pktFlag(dtmp,13);
        //sprintf(contents, "%d %d %d %d\n",cmd, seq,time,flat);

       // ALOGI(0, "222222222222222222222222 rescvcall seq %d cmd %d time %d",
         //     seq, cmd,time);

if(!hashead){
    vpts=time;
    hashead=true;
}

     //   fwrite(contents, 1, strlen(contents), f2);
       // fwrite((char *)hbgttmp->data + hbgttmp->start, 1, hbgttmp->len, f2);

        total += (hbgttmp->len);
        hcntmp=hcntmp->next;
    }
    fclose(f2);







    uint32_t size = mfc->size;
    hg_chain_node *hcn = mfc->hct.left;
    App::onRecvVieData_Callback(ISVIDEO, hcn, size-RTP_PACKET_HEAD_SIZE,vpts);

}

///////////////////////////////////////

void VideoHandle::SendData(void *data, uint32_t size, uint16_t time) {
    ALOGI(0, "3333333333333333333 sendsend %d", size);

    if (lasttime != 0 && lasttime == time) {
        assert(0);
    }
    lasttime = time;
    hg_chain_t hct;
    /*
    FILE *f2 = fopen(APP_ROOT_"0.data", "a");
    fwrite(data, 1, size, f2);
    fclose(f2);
*/
    handle.Channel.Drive(nullptr, 0);
    int mfcsize = HgNetabstract::CreateFrameStr(handle.fragCache, &hct, hgapp->ssrc,
                                                (int) ENU_PLTYPEVIDEO, false,
                                                (int) CMD_DEFAULT, time, (uint8_t *) data, size,
                                                HANDLE_MAX_FRAME_SLICE,-1);

    media_frame_chain mfc;

    HandleCom::framebufSet(&mfc,time, &hct, mfcsize, &handle);
    HgSessionC *sesstm = nullptr;
    if (hgapp->huageConn != nullptr) {
        sesstm = hgapp->huageConn->data;
    }
    if (sesstm == nullptr) {
        return;
    }
    HgNetabstract::PreSendData(hgapp->hgcc->hnab, &mfc, hgapp->ssrc, (int) ENU_PLTYPEVIDEO,
                               0, sesstm, false, nullptr,0);
}


