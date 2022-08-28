//
// Created by ajl on 2021/11/11.
//

#ifndef HUAGERTP_APP_HUGE_H
#define HUAGERTP_APP_HUGE_H

#include "common/extern.h"
#include "stdint.h"
#include "common/hg_iocp.h"
#include "threads/hg_worker.h"
#include "common/hg_pipe.h"
#include "../app/app.h"
#include "common/hg_netabstract.h"
#include "hg_com.h"

extern HgWorker *hgworkers[WORKERNUM];

class HgTcpabstract;

class HgSessionC;

struct sockaddr_in;

typedef void( *HandlWriteFr)(void *ctx, void *data);

class HgClient {
public:
    HgSessionC *sess= nullptr;
    HgNetabstract *hnab= nullptr;

    HgClient(char *ip, int port);

    void *audioctx= nullptr;
    void *videoctx= nullptr;
    void *textctx= nullptr;

    void StartRun();


    static void
    RecvCallback(PLTYPE pltype, void *ctx, media_frame_chain *mfc, HgSessionC *sess);


    void
    (*recvaudioCallback)(void *ctx, media_frame_chain *mfc, HgSessionC *sess)= nullptr;

    void
    (*recvvideoCallback)(void *ctx, media_frame_chain *mfc, HgSessionC *sess)= nullptr;

    void
    (*recvtextCallback)(void *ctx, media_frame_chain *mfc, HgSessionC *sess)= nullptr;

    static bool
    Verify_Auth(void *ctx,HgSessionC *sess, int fd);
    bool (*verify_auth)(void *ctx,HgSessionC *sess, int fd);

    static void
    AsyncLogin(void *ctx,media_frame_chain *mfc, HgSessionC *sess);
    void (*asyncLogin)(void *ctx, media_frame_chain *mfc, HgSessionC *sess)= nullptr;


};


#endif //HUAGERTP_APP_HUGE_H
