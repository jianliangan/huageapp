//
// Created by ajl on 2021/1/18.
//

#include "app.h"
#include "../net/protocol/core/tools.h"
#include "../net/protocol/core/hg_queue.h"

#include "../base/json.hpp"
#include "../net/protocol/hgmain.h"

App *hgapp= nullptr;
using namespace std;
onLogined App::onLogin_Callback;
onRecvData App::onRecvVieData_Callback;
onRecvDataA App::onRecvAudData_Callback;

App::App(void) {
    startts=0;
    islittleend=true;
    serverip= nullptr;
    serverport=0;
    aHandle= nullptr;
    tHandle= nullptr;
    vHandle= nullptr;
    loginNum=0;
    ssrc = 0;
    roomid = 0;
    memset(uname,'\0',sizeof(uname));
    loginBuf = new uint8_t[512];
    pthread_mutex_init(&syncEventmtx, NULL);
    huageConn = nullptr;
    hgcc= nullptr;
}

int App::CreateEnterRoom(int roomid, SessionC *sess) {

    unordered_map <string, nlohmann::json> loginParams{{"name",   "ajl"},
                                                     {"roomid", roomid},

                                                     {"chan",   2},
                                                     {"deep",   16},
                                                     {"samrate",   8000},
                                                     {"frlen",  160},

                                                     {"codec",  AUDIO_G711},
                                                     {"uid",    std::to_string(ssrc)}};
    nlohmann::json obj = nlohmann::json(loginParams);
    //string logindump = obj.dump();
    tHandle->SendData(obj, "createenterroom",false, TextHandle::CreateEnterRoomResCB);

    return 0;
}

void App::DoLogin() {

    unordered_map <string, string> loginParams{{"name",   std::string(uname)},
                                               {"roomid", std::to_string(roomid)},
                                               {"uid",    std::to_string(ssrc)}
                                               };
    nlohmann::json obj = nlohmann::json(loginParams);
    string logindump = obj.dump();
    tHandle->SendData(obj, "login",true, TextHandle::LoginResCB);

}

