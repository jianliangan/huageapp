//
// Created by ajl on 2021/9/2.
//

#include "audio_encoder.h"
//
// Created by ajl on 2021/1/17.
//
#include "audio_source.h"
#include "audio_encoder.h"
#include "app.h"
#include "../base/g711.h"

AudioEncoder::AudioEncoder(audio_config_t *recconfig) {

    input_size = 0;
    lasttime=0;

    uint32_t err = 0;
    finished = false;
    captureconf.channels = recconfig->channels;
    captureconf.samplesrate = recconfig->samplesrate;
    captureconf.deep = recconfig->deep;
    captureconf.frameperbuf = recconfig->frameperbuf;
    err = InitEncoder();
    //fragCache = myPoolCreate(2046);
    if (err != 0) {
        ALOGI(0, "audio encoder init err");
        return;
    }
    finished = true;
    framerawsize = captureconf.frameperbuf * captureconf.deep * 1 >> 3;
}

//int AudioEncoder::StartRun(){
//
//    uint32_t err;
//    err = pthread_create(&eventThread, NULL, HandleEncoder, this);
//    if (err != 0) {
//        ALOGI(0, "audio encoder thread err");
//        return -1;
//    }
//
//    finished = true;
//    return 0;
//}
free_frame_a *AudioEncoder::GetFreeFrame() {
    free_frame_a *re = nullptr;
    if (framefreea != nullptr) {
        re = framefreea;
        framefreea = framefreea->next;

    } else {

        re = (free_frame_a *) malloc(sizeof(free_frame_a) + framerawsize);
        re->init();
        re->data = (char *) re + sizeof(free_frame_a);

    }

    return re;
}

void AudioEncoder::PushDataIn(ev_audio_t *eat, uint16_t vpts) {
    uint16_t time=eat->pts;
    if (lasttime != 0 && lasttime == time) {
        assert(0);
    }
    lasttime = time;
    ALOGI(0, "aaaaaaaaaaaaaaaaaaaaaaaa2222abbbbbbbbbbbbbbbbb %d", framerawsize);
    assert(eat->size == framerawsize);
    free_frame_a *ffa = GetFreeFrame();
    memcpy(ffa->data, eat->data, eat->size);
    ffa->size=eat->size;
    int lens = sizeof(hgtEvent) + sizeof(audio_frame_raw_pa);
    char hgteventptr[lens];
    hgtEvent *hgtevent = (hgtEvent *) hgteventptr;
    audio_frame_raw_pa *afrp = (audio_frame_raw_pa *) (hgteventptr + sizeof(hgtEvent));
    afrp->data = ffa;
    afrp->vpts = vpts;
    afrp->apts = eat->pts;


    hgtevent->handle = AudioEncoder::HandleEncoder;
    hgtevent->i = 25;
    hgtevent->psize = lens - sizeof(hgtEvent);
    hgtevent->ctx = this;
    HgWorker *hgworker = HgWorker::GetWorker(0);
    hgworker->WriteChanWor(hgteventptr, lens);
}

void AudioEncoder::HandleEncoder(void *pth, void *ctx, void *params, int psize) {
    AudioEncoder *audioenco = (AudioEncoder *) ctx;
    audio_frame_raw_pa *afrp = (audio_frame_raw_pa *) params;
    free_frame_a *ffa = afrp->data;
    int16_t *audiov = (int16_t *)ffa->data;
    int audiopts = afrp->apts;
    int videopts=afrp->vpts;


    // int framerawsize=audioenco->captureconf.frameperbuf*audioenco->captureconf.deep*audioenco->captureconf.channels>>3;
    int audio_frame_out_len = audioenco->framerawsize / 2;
    uint8_t audio_frame_out[audio_frame_out_len];
    //这个过程需要加hash解决，可以直接用1024个桶，或者用map，来处理多个缓存的问题，相当于隐约的流


    for (int i = 0; i < audio_frame_out_len; i++) {
        audio_frame_out[i] = MuLaw_Encode(*(audiov + i));
    }
/////////////

///////////////////
    uint8_t msobjencodes[MSG0_MAX_DE];
    MsgObj0 msobj0;
    msobj0.ssrc=hgapp->ssrc;
    msobj0.apts=audiopts;
    msobj0.vpts=videopts;


    char aaa[1000];
    sprintf(aaa,"ssrc %d apts %d vpts %d\n",hgapp->ssrc,audiopts,videopts);
    FILE *f2 = fopen(APP_ROOT_"test0.data", "a+");
    fwrite(aaa,1,strlen(aaa),f2);
    fclose(f2);


    int msenlen=MsgObj0::getenlen(&msobj0,1);
    if(msenlen<MSG0_MAX_DE){
        MsgObj0::encode(msobjencodes,&msobj0,1);
        hgapp->aHandle->SendData(msobjencodes, msenlen, 0,1);
    }
    hgapp->aHandle->SendData(audio_frame_out, audio_frame_out_len, audiopts,0);

    WriteFree(audioenco, ffa);
    // }

}
void AudioEncoder::FreeByChan(void *pth, void *ctx, void *params, int psize) {
    AudioEncoder *aen=(AudioEncoder *)ctx;
    free_frame_a **ptmp = (free_frame_a **) params;
    free_frame_a *ffca=*ptmp;
    ffca->next=aen->framefreea;
    aen->framefreea=ffca;
}

void AudioEncoder::WriteFree(void *ctx, void *data) {
    AudioEncoder *aen = (AudioEncoder *) ctx;

    int lens = sizeof(hgtEvent) + sizeof(void *);
    char tmp[lens];
    hgtEvent *hgtevent = (hgtEvent *) tmp;
    hgtevent->handle = AudioEncoder::FreeByChan;
    hgtevent->i=21;
    hgtevent->ctx = ctx;
    hgtevent->psize = lens-sizeof(hgtEvent);
    *((void **) ((char *) tmp + sizeof(hgtEvent))) = data;
    HgWorker *hgworker = HgWorker::GetWorker(0);
    hgworker->WriteChanWor(tmp, lens);
}


int AudioEncoder::InitEncoder() {
    //下面是初始化缓冲区
    input_size = 512;
    return 0;
}

