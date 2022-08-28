//
// Created by ajl on 2021/6/23.
//

#include "audio_player.h"
#include "audio_recorder.h"

void AudioPlayer::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx) {
    AudioPlayer *audioPlay = (AudioPlayer *) ctx;
    //audioPlay->callback_f(2, 2);
    //int64_t aaa=hgetSysTimeMicros()/1000-audioPlay->tmp;
   // if(aaa>20)
   // ALOGI(0,"time11111----:%lld",aaa);
   // audioPlay->tmp=hgetSysTimeMicros()/1000;
    uint32_t produce = audioPlay->produceCount;
    uint32_t consume = audioPlay->consumeCount;

   int index = consume % DEVICE_PLAY_BUFFER_QUEUE_LEN;
    SLresult result;

/*
    FILE *f2 = fopen("/data/data/org.libhuagertp.app/2.data", "a");
    fwrite(audioPlay->buffer_o[index]->data, 1, audioPlay->bufSize, f2);
    fclose(f2);*/



    result = (*audioPlay->playBufferQueueItf)->Enqueue(audioPlay->playBufferQueueItf,
                                                       audioPlay->buffer_o[index]->data,
                                                       audioPlay->bufSize);

    SLASSERT(result);

    if (!(uint32Sub(produce, consume) ==1)) {
        consume = uint32Add(consume, 1);
        audioPlay->currentsize=0;
        audioPlay->consumeCount=consume;
    }else{
        memset(audioPlay->buffer_o[index]->data,0x00000000,audioPlay->bufSize);
    }
}

AudioPlayer::AudioPlayer(SampleFormat *sampleFormat, SLEngineItf slEngine)
        : callback_f(nullptr) {
    SLresult result;
    assert(sampleFormat);

    consumeCount=0;//当前要消费的在的数，将来改成cache索引好了
    produceCount=0xffffffff;//当前要存储在的数
    result = (*slEngine)
            ->CreateOutputMix(slEngine, &outputMixObjectItf, 0, NULL, NULL);
    SLASSERT(result);

    // realize the output mix
    result =
            (*outputMixObjectItf)->Realize(outputMixObjectItf, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_PLAY_BUFFER_QUEUE_LEN};

    SLAndroidDataFormat_PCM_EX format_pcm;
    ConvertToSLSampleFormat(&format_pcm, sampleFormat);
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX,
                                          outputMixObjectItf};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    /*
     * create fast path audio player: SL_IID_BUFFERQUEUE and SL_IID_VOLUME
     * and other non-signal processing interfaces are ok.
     */
    SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioPlayer(
            slEngine, &playerObjectItf, &audioSrc, &audioSnk,
            sizeof(ids) / sizeof(ids[0]), ids, req);
    SLASSERT(result);

    // realize the player
    result = (*playerObjectItf)->Realize(playerObjectItf, SL_BOOLEAN_FALSE);
    SLASSERT(result);

    // get the play interface
    result = (*playerObjectItf)
            ->GetInterface(playerObjectItf, SL_IID_PLAY, &playItf_o);
    SLASSERT(result);

    // get the buffer queue interface
    result = (*playerObjectItf)
            ->GetInterface(playerObjectItf, SL_IID_BUFFERQUEUE,
                           &playBufferQueueItf);
    SLASSERT(result);

    // register callback on the buffer queue
    result = (*playBufferQueueItf)
            ->RegisterCallback(playBufferQueueItf, ProcessSLCallback, this);
    SLASSERT(result);

    result = (*playItf_o)->SetPlayState(playItf_o, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);

}

AudioPlayer::~AudioPlayer() {


    // destroy buffer queue audio player object, and invalidate all associated
    // interfaces
    if (playerObjectItf != NULL) {
        (*playerObjectItf)->Destroy(playerObjectItf);
    }
    // Consume all non-completed audio buffers
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObjectItf) {
        (*outputMixObjectItf)->Destroy(outputMixObjectItf);
    }
}


SLresult AudioPlayer::Start(void) {
    SLuint32 state;
    SLresult result = (*playItf_o)->GetPlayState(playItf_o, &state);
    if (result != SL_RESULT_SUCCESS) {
        return SL_BOOLEAN_FALSE;
    }
    if (state == SL_PLAYSTATE_PLAYING) {
        return SL_BOOLEAN_TRUE;
    }


    SLASSERT(result);
    result = (*playItf_o)->SetPlayState(playItf_o, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);
    result = (*playBufferQueueItf)->Enqueue(playBufferQueueItf, buffer_o[0]->data, bufSize);
    SLASSERT(result);
    result = (*playItf_o)->SetPlayState(playItf_o, SL_PLAYSTATE_PLAYING);
    SLASSERT(result);
    return SL_BOOLEAN_TRUE;
}

//回头需要加缓冲，所有的缓冲都是解决写快，读慢的问题
void AudioPlayer::writeBuf(uint8_t *buf, int size) {
    uint32_t produce = produceCount;
    bool isfirst=false;
    if(produce==0xffffffff){
        isfirst=true;
        produce=0;
    }
    int index = produce % DEVICE_PLAY_BUFFER_QUEUE_LEN;
    if (size > bufSize) {
        assert(false);
    }

    uint32_t consume = consumeCount;
    if (!isfirst&&uint32Sub(consume,produce) == 1) {
        ALOGI(0,"yyyyyyyyyyyyyyyyyyyyyyyyuuuuuuuuu produce %lld consume %lld",produce,consume);
        return;
    }
    int16_t *cur=(int16_t *)buf;
    int16_t *dist=(int16_t *)buffer_o[index]->data;
    int cout16=size/2;
  for(int i=0;i<cout16;i++){
      *dist=*(cur+i);
      *(dist+1)=*dist;
      dist+=2;
  }
    produce=uint32Add(produce, 1);
    produceCount=produce;
}

void AudioPlayer::createBuffer(int len, int size) {
    playSample **buf;
    buf = new playSample *[len];

    for (int i = 0; i < len; i++) {
        playSample *ps = new playSample();
        ps->data = new uint8_t[size];
        buf[i] = ps;
    }
    buffer_o = buf;
    bufSize = size;
}

void AudioPlayer::Stop(void) {
    SLuint32 state;

    SLresult result = (*playItf_o)->GetPlayState(playItf_o, &state);
    SLASSERT(result);

    if (state == SL_PLAYSTATE_STOPPED) return;


    result = (*playItf_o)->SetPlayState(playItf_o, SL_PLAYSTATE_STOPPED);
    SLASSERT(result);
    (*playBufferQueueItf)->Clear(playBufferQueueItf);


}

void AudioPlayer::RegisterCallback(JNI_AUDIO_CALLBACK cb, void *ctx) {
    callback_f = cb;
    isfinish = true;
}
