//
// Created by ajl on 2022/1/18.
//

#include "handle_com.h"
#include "../net/threads/hg_worker.h"
HandleCom::HandleCom(){
    fragCache = myPoolCreate(HANDLE_MAX_POOL);
}
void HandleCom::framebufSet(media_frame_chain *mfc,uint16_t time,hg_chain_t *hct,int length,void *ctx){
    mfc->size = length;
    mfc->hct = *hct;
    mfc->pts=time;
    mfc->sfree.params = nullptr;
    mfc->sfree.ctx = nullptr;
    mfc->sfree.freehandle = nullptr;

    hg_chain_node *hcnR=mfc->hct.right;
    hg_Buf_Gen_t *hbgtR=(hg_Buf_Gen_t *)hcnR->data;
    hbgtR->sfree.ctx=ctx;
    hbgtR->sfree.params=hct->left;

    HandleCom *hcc=(HandleCom *)ctx;

    //ALOGI(0,"tttttttttttttttttttt free pool rrr %ld %ld",hcc->fragCache,hct->left);
    hbgtR->sfree.freehandle=HandleCom::WriteFree;
}
void HandleCom::WriteFree(void *ctx, void *data) {
    HandleCom *huc = (HandleCom *) ctx;
    int lens=sizeof(hgtEvent)+sizeof(void *);
    char tmp[lens];
    hgtEvent *hgtevent=(hgtEvent *)tmp;

    hgtevent->handle = HandleCom::FreeByChan;
    hgtevent->i=0;
    hgtevent->ctx = ctx;
    hgtevent->psize=sizeof(data);
    *((void **)((char *)tmp+sizeof(hgtEvent)))=data;
    huc->Channel.WriteChan(tmp,lens,0);
}

void HandleCom::FreeByChan(void *pth, void *ctx, void *params, int psize) {
    ALOGI(0,"HandleCom::FreeByChan  0");
    hg_chain_node **ptmp = (hg_chain_node **) params;
    hg_chain_node *hcn=*ptmp;
    HandleCom *hcl = (HandleCom *) ctx;
    while (hcn != nullptr) {
        ALOGI(0,"HandleCom::FreeByChan  1-11");
        myPoolFree(hcl->fragCache, (uint8_t *) hcn);
        ALOGI(0,"HandleCom::FreeByChan  1-12 %ld",hcn);
        hcn = hcn->next;
    }
    ALOGI(0,"HandleCom::FreeByChan  2");
}
MsgObj0::MsgObj0(){
    this->vpts=0;
    this->apts=0;
    this->ssrc=0;
}
int MsgObj0::getdelen(uint8_t *source,int size){
    assert(size>2);
    if(source[0]==0){
        return source[1];
    }
    return 0;
}
int MsgObj0::getenlen(MsgObj0 *source,int size){
    return sizeof(MsgObjHead)+size*sizeof(MsgObj0);
}
void MsgObj0::encode(uint8_t *dist,MsgObj0 *source,int ssize){
    dist[0]=0;
    dist[1]=ssize;
    int offset=sizeof(MsgObjHead);
    int vsize=sizeof(MsgObj0);
    for(int i=0;i<ssize;i++){
        offset=offset+i*vsize;
        toolsIntBigEndian(dist+offset, (uint32_t)(source+i)->ssrc, 32);
        toolsIntBigEndian(dist+offset+4, (uint32_t)(source+i)->apts, 16);
        toolsIntBigEndian(dist+offset+4+2, (uint32_t)(source+i)->vpts, 16);
    }
}
void MsgObj0::decode(uint8_t *source,int ssize,MsgObj0 *dist,int dsize){
    assert(source[0]==0);
    int offset=sizeof(MsgObjHead);
    int vsize=sizeof(MsgObj0);
    for(int i=0;i<dsize;i++){
        offset=offset+i*vsize;
        toolsIntBigEndianV(source+offset, (uint8_t *)&((dist+i)->ssrc), 32);
        toolsIntBigEndianV(source+offset+4, (uint8_t *)&((dist+i)->apts), 16);
        toolsIntBigEndianV(source+offset+4+2, (uint8_t *)&((dist+i)->vpts), 16);
    }
}