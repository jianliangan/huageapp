//
// Created by ajl <429200247@qq.com> on 2020/12/20.
//

#include "text_handle.h"
#include "../net/protocol/core/tools.h"
#include "app.h"
#include "../net/protocol/hg_channel_stru.h"
#include "../net/hg_client.h"
#include "handle_com.h"

using namespace std;
uint32_t Request::unique;

Request::Request() {
    IncrUniq();

}

Request::~Request() {

}

void Request::IncrUniq() {
    unique = (unique + 1) & 0xffffffff;
}

TextHandle::TextHandle() {
    uint32_t err;
    //pthread_mutex_init(&reuestMtx, NULL);


}

void TextHandle::SendData(nlohmann::json body, string uri, bool isfirst, text_funcCb cb) {
    uint32_t *ts= nullptr;

    Request *request = new Request();
    request->ctx = this;
    request->callback = cb;
    uint8_t *data= nullptr;
    uint32_t unique = Request::unique;

    //hg_pthread_lock(&reuestMtx);
    if (requests.find(unique) != requests.end()) {
        //hg_pthread_unlock(&reuestMtx);
    }
    requests[unique] = request;
    int pp=requests.size();
    //hg_pthread_unlock(&reuestMtx);


    nlohmann::json reqobj;
    reqJsoHead(unique,uri,reqobj,body);


    string reqBody = reqobj.dump();
    uint32_t length = 0;
    uint32_t reqTotalLen = 0;

    //requestHead rqhead;
    //rqhead.uniqId = unique;
    length = reqBody.length();

    hg_chain_t hct;

    handle.Channel.Drive(nullptr, 0);
    int mfcsize = HgNetabstract::CreateFrameStr(handle.fragCache, &hct, hgapp->ssrc,
                                         (int) ENU_PLTYPETEXT, isfirst,(int) CMD_DEFAULT, 0,(uint8_t *) reqBody.c_str(),length,HANDLE_MAX_FRAME_SLICE,-1);

    media_frame_chain mfc;

    HandleCom::framebufSet(&mfc,0, &hct, mfcsize, &handle);
    HgSessionC *sesstm = nullptr;
    if (hgapp->huageConn != nullptr) {
        sesstm = hgapp->huageConn->data;
    }
    HgNetabstract::PreSendData(hgapp->hgcc->hnab, &mfc, hgapp->ssrc,
                               (int) ENU_PLTYPETEXT,
                               isfirst, sesstm, false, nullptr,0);

}
void TextHandle::SendCallback(void *pth, void *ctx, void *params, int psize){

}
bool TextHandle::Verify_auth(void *ctx,HgSessionC *sess,int fd) {
    //判断是否是服务器发来的
    if (sess != nullptr && hgapp->huageConn->status == SESSLOGIN) {
        return true;
    } else {
        return false;
    }
}

//;
void TextHandle::AsyncLogin(void *ctx, media_frame_chain *mfc,
                            HgSessionC *sess) {
    RecvCallback(ctx, mfc, sess);
}

void TextHandle::RecvCallback(void *ctx, media_frame_chain *mfc,
                              HgSessionC *sess) {
    TextHandle *context = (TextHandle *) ctx;
    int size = mfc->size;
    uint8_t *buffer= nullptr;
    if (size < 4) {
        ALOGI(0, "server  error %d", size);
        return;
    }

    char jsonblock[MAX_TEXT_BUF_SIZE]={0};

    hg_Buf_Gen_t *hbgt= nullptr;
    int total = 0;
    hg_chain_node *hcn = mfc->hct.left;
    while (hcn != nullptr) {
        hbgt = (hg_Buf_Gen_t *) hcn->data;
        memcpy((char *) jsonblock + total, (char *) hbgt->data + hbgt->start, hbgt->len);
        total += hbgt->len;
        hcn = hcn->next;
    }


    uint8_t *frameLine = (uint8_t *) jsonblock;
    uint32_t resLen = mfc->size-RTP_PACKET_HEAD_SIZE;
//text协议自定义的话，不需要太复杂的头，弄个简单的就行，另外流一定要有头的，方便解析

    nlohmann::json resbody = nlohmann::json::parse(std::string((char *) frameLine, resLen), nullptr,false);
    if(resbody.is_discarded()){
        ALOGI(0, "server resbody recv format error ");
        return;
    }

    uint32_t requestUnique;
    auto ru=resbody.find("i");
    if(ru==resbody.end()){
ALOGI(0,"json at i error ");
return ;
    }
        requestUnique = *ru;

    unordered_map<uint32_t, Request *>::iterator reqit;
   int pp=context->requests.size();
    if(context->requests.size()>0) {
        reqit = context->requests.find(requestUnique);
        if (reqit == hgapp->tHandle->requests.end()) {
            return;
        }
        Request *req = reqit->second;
        req->callback(req, resbody, sess);
    }
}

void TextHandle::CreateEnterRoomResCB(Request *req, nlohmann::json body, HgSessionC *sess) {
    TextHandle *th = (TextHandle *) req->ctx;
    auto bodyit=body.find("b");
    if(bodyit==body.end()){
        ALOGI(0,"json error ");
 return;
    }
    std::string bodystr=*bodyit;
    if (bodystr == "ok") {
        //(App *) app = th->GetApp();
        //app->Logined();
    }
}

void TextHandle::LoginResCB(Request *req, nlohmann::json body, HgSessionC *sess) {
    TextHandle *th = (TextHandle *) req->ctx;
    string re;
    auto bodyit=body.find("b");
    if(bodyit==body.end()){
        ALOGI(0,"json error ");
        return ;
    }
   re=*bodyit;
    if ( re== "ok") {
        App *app = (App *) th->GetApp();

        hgapp->huageConn = (SessionC *)sess->data;
        hgapp->huageConn->ssrc = hgapp->ssrc;
        hgapp->huageConn->status = SESSLOGIN;
        hgapp->huageConn->data = sess;
        ALOGI(0, " logined response str");
        //登陆成功，以后还可能会拿到sid，用来回调鉴权
        App::onLogin_Callback();


    }
}

void *TextHandle::GetApp() {
    return hgapp;
}


TextHandle::~TextHandle() {

}