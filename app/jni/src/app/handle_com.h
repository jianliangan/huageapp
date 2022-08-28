//
// Created by ajl on 2022/1/18.
//

#ifndef HUAGERTP_APP_HANDLE_COM_H
#define HUAGERTP_APP_HANDLE_COM_H
#include "../net/protocol/hg_channel_stru.h"
#include "../net/protocol/hgsession.h"
#include "../net/threads/hg_channel.h"
#define HANDLE_MAX_POOL 4098
#define HANDLE_MAX_FRAME_SLICE 2048
class MsgObjHead{
    uint8_t type;
    uint8_t count;
};
class MsgObj0{
public:
    uint32_t ssrc;
    uint16_t vpts;
    uint16_t apts;
    MsgObj0();
    static int getdelen(uint8_t *source,int size);
    static int getenlen(MsgObj0 *source,int size);
    static void encode(uint8_t *dist,MsgObj0 *source,int ssize);
    static void decode(uint8_t *source,int ssize,MsgObj0 *dist,int dsize);
};


class HandleCom {
public:
    HgChannel Channel;
    myPool *fragCache;

    HandleCom();
    static void WriteFree(void *ctx, void *data);
    static void FreeByChan(void *pth, void *ctx, void *params, int psize);

    static void framebufSet(media_frame_chain *mfc,uint16_t time,hg_chain_t *hct,int length,void *ctx);
};


#endif //HUAGERTP_APP_HANDLE_COM_H
