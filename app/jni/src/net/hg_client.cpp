//
// Created by ajl on 2021/11/11.
//

#include "hg_client.h"
#include "protocol/hgsession.h"

#include "common/hg_pipe.h"
#include "protocol/rtp_packet.h"
#include "threads/hg_worker.h"
#include "protocol/hgfdevent.h"
HgClient::HgClient(char *ip, int port) {
    hnab=new HgNetabstract();
    HgUdpabstract *aUdpinstan = new HgUdpabstract();
    HgTcpabstract *aTcpinstan = new HgTcpabstract();

    aUdpinstan->SetServer(ip, port);
    aTcpinstan->SetServer(ip, port + 1);

    hnab->aUdpinstan=aUdpinstan;
    hnab->aTcpinstan=aTcpinstan;

    hnab->isclient=true;
    hnab->aTcpinstan->isclient=true;
    hnab->aUdpinstan->isclient=true;


    HgSessionC::sess_cb=SessionC::sessFree;
    HgSessBucket *hgsessbu= nullptr;
    int sessnum=50;
    for (int i = 0; i < WORKERNUM;
         i++) {
        hgworkers[i] = new HgWorker();
        hgsessbu = new HgSessBucket();
        HgSessionC::sessInit(sessnum, &hgsessbu);
        SessionC *sesstmp=SessionC::sessInit(sessnum);

        HgSessionC *hgsesstmp= hgsessbu->sessFreePtr;
        for(int j=0;j<sessnum;j++){
            sesstmp+=1;
            sesstmp->data=hgsesstmp;
            hgsesstmp->data=sesstmp;
            hgsesstmp=hgsesstmp->fnext;
        }
        hgworkers[i]->sess_s = hgsessbu;
    }


}

void HgClient::StartRun() {

    HgIocp *iocp = new HgIocp();
    HgFdEvent::fdEvChainInit(&iocp->fdchainfree, 10);

    HgUdpabstract *aUdpinstan=hnab->aUdpinstan;
    aUdpinstan->recvCallback = RecvCallback;
    aUdpinstan->verify_auth = Verify_Auth;
    aUdpinstan->asyncLogin = AsyncLogin;
    aUdpinstan->ver_recv_ctx = this;

    HgTcpabstract *aTcpinstan=hnab->aTcpinstan;
    aTcpinstan->recvCallback = RecvCallback;
    aTcpinstan->verify_auth = Verify_Auth;
    aTcpinstan->asyncLogin = AsyncLogin;
    aTcpinstan->ver_recv_ctx = this;


    /////
    aTcpinstan->iocp = iocp;
    aUdpinstan->iocp = iocp;
/////
    aUdpinstan->Init();
    aTcpinstan->Init();
    iocp->StartRun();

}

void HgClient::RecvCallback(PLTYPE pltype, void *ctx, media_frame_chain *mfc,
                            HgSessionC *sess) {
    HgClient *uc = (HgClient *) ctx;
    if (pltype == ENU_PLTYPEAUDIO) {
        if (uc->audioctx == nullptr) {
            return;
        }
        uc->recvaudioCallback(uc->audioctx, mfc, sess);
    } else if (pltype == ENU_PLTYPEVIDEO) {
        if (uc->videoctx == nullptr) {
            return;
        }
        uc->recvvideoCallback(uc->videoctx, mfc, sess);
    } else if (pltype == ENU_PLTYPETEXT) {
        if (uc->textctx == nullptr) {
            return;
        }
        uc->recvtextCallback(uc->textctx, mfc, sess);
    }
}

void HgClient::AsyncLogin(void *ctx,media_frame_chain *mfc, HgSessionC *sess) {
    HgClient *uc = (HgClient *) ctx;
         uc->asyncLogin(uc->textctx, mfc, sess);
}
bool HgClient::Verify_Auth(void *ctx,HgSessionC *sess, int fd) {
    HgClient *uc = (HgClient *) ctx;
    return uc->verify_auth(uc->textctx, sess, fd);
}

