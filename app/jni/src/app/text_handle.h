//
// Created by ajl <429200247@qq.com> on 2020/12/20.
//loop负责采集接受
//

#ifndef ANDROID_PROJECT_TEXTHANDLE_H
#define ANDROID_PROJECT_TEXTHANDLE_H

#define MAX_TEXT_BUF_SIZE 1024*4  //一个命令包最长的值
#include <string>
#include "../net/protocol/core/tools.h"


#include "../net/protocol/huage_sendstream.h"
#include "../net/protocol/huage_recvstream.h"
#include "../net/threads/hg_channel.h"
#include "session.h"

#include "../base/json.hpp"
#include "handle_com.h"
//支持最大的接收，发送数据是4k，以后再改
typedef struct requestHead{
    uint32_t uniqId;
} requestHead;
class Request{
public:

    static uint32_t unique;

    SessionC *sess= nullptr;
    uint32_t resBoSize=0;
    void *ctx= nullptr;//the object sended request
    Request();
    ~Request();
    static void IncrUniq();
    void(*callback)(Request *ev,nlohmann::json body,HgSessionC *sess)= nullptr;
};

typedef void (*text_funcCb)(Request *req,nlohmann::json body,HgSessionC *sess);
class TextHandle {
public:
    HandleCom handle;
    bool finished;
    std::unordered_map<unsigned int, Request *> requests;
    //对端地址，考虑放到别处
    FILE *f2;
    TextHandle();
    ~TextHandle();

    void SendData(nlohmann::json body,std::string uri,bool isfirst,text_funcCb cb);
    static void SendCallback(void *pth, void *ctx, void *params, int psize);
    static void AsyncLogin(void *ctx, media_frame_chain *mfc, HgSessionC *sess);
    void *GetApp();

    static bool Verify_auth(void *ctx,HgSessionC *sess,int fd);
    static void RecvCallback(void *ctx,media_frame_chain *mfc, HgSessionC *sess);
    //接收
    static void LoginResCB(Request *req, nlohmann::json body,HgSessionC *sess);
    static void CreateEnterRoomResCB(Request *req, nlohmann::json body,HgSessionC *sess);

};


#endif //ANDROID_PROJECT_textHandle_H
