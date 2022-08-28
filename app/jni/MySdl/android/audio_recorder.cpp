/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include <cstdlib>
#include "audio_recorder.h"
#include "audio_common.h"
#include "ex_hgaudio_common.h"

void AudioRecorder::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx) {
    AudioRecorder *audioRec = (AudioRecorder *) ctx;
    uint32_t curpts = audioRec->produceCount;
    bool isfirst=false;
    if(curpts==0xffffffff){
        isfirst=true;
        curpts=0;
    }
    uint32_t lastpts = uint32Sub(curpts, 1);

    int curindex = curpts % DEVICE_RECODE_BUFFER_QUEUE_LEN;
    if (!isfirst) {
        ev_audio_t eat;
        int tmpindex = lastpts % DEVICE_RECODE_BUFFER_QUEUE_LEN;

        eat.data = audioRec->buffer_o[tmpindex]->data;
        eat.size = audioRec->bufSize;
        eat.pts = lastpts;
        //ALOGI(0,"aaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbb ******  %d %d %ld",lastpts,curpts,(int64_t)audioRec);
        audioRec->callback_f(&eat);
    }


    (*audioRec->recBufQueueItf)->Enqueue(audioRec->recBufQueueItf,
                                         audioRec->buffer_o[curindex]->data,
                                         audioRec->bufSize);

    curpts = uint32Add(curpts, 1);
    audioRec->produceCount=curpts;

    // SLASSERT(result);
}


AudioRecorder::AudioRecorder(SampleFormat *sampleFormat, SLEngineItf slEngine)
        : callback_f(nullptr) {
    SLresult result;

    SLAndroidDataFormat_PCM_EX format_pcm;
    produceCount=0xffffffff;//准备写到这个位置
    ConvertToSLSampleFormat(&format_pcm, sampleFormat);

    // configure audio source
    SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
                                      SL_IODEVICE_AUDIOINPUT,
                                      SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
    SLDataSource audioSrc = {&loc_dev, NULL};

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_RECODE_BUFFER_QUEUE_LEN};

    SLDataSink audioSnk = {&loc_bq, &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                 SL_IID_ANDROIDCONFIGURATION};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*slEngine)->CreateAudioRecorder(
            slEngine, &recObjectItf, &audioSrc, &audioSnk,
            sizeof(id) / sizeof(id[0]), id, req);
    SLASSERT(result);

    // Configure the voice recognition preset which has no
    // signal processing for lower latency.
    SLAndroidConfigurationItf inputConfig;
    result = (*recObjectItf)
            ->GetInterface(recObjectItf, SL_IID_ANDROIDCONFIGURATION,
                           &inputConfig);
    if (SL_RESULT_SUCCESS == result) {
        SLuint32 presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
        (*inputConfig)
                ->SetConfiguration(inputConfig, SL_ANDROID_KEY_RECORDING_PRESET,
                                   &presetValue, sizeof(SLuint32));
    }
    result = (*recObjectItf)->Realize(recObjectItf, SL_BOOLEAN_FALSE);

    result =
            (*recObjectItf)->GetInterface(recObjectItf, SL_IID_RECORD, &recItf_o);


    result = (*recObjectItf)
            ->GetInterface(recObjectItf, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                           &recBufQueueItf);


    result = (*recBufQueueItf)
            ->RegisterCallback(recBufQueueItf, ProcessSLCallback, this);

}

bool AudioRecorder::Start(void) {

    SLresult result;
    // in case already recording, stop recording and clear buffer queue
    result = (*recItf_o)->SetRecordState(recItf_o, SL_RECORDSTATE_STOPPED);

    result = (*recBufQueueItf)->Clear(recBufQueueItf);


    result = (*recBufQueueItf)->Enqueue(recBufQueueItf, buffer_o[0]->data, bufSize);
    result = (*recItf_o)->SetRecordState(recItf_o, SL_RECORDSTATE_RECORDING);
    //ALOGI(0,"time000:%lld",hgetSysTimeMicros()/1000);

    return (result == SL_RESULT_SUCCESS ? true : false);
}

void AudioRecorder::createBuffer(int len, int size) {
    recSample **buf;
    buf = new recSample *[len];

    for (int i = 0; i < len; i++) {
        recSample *ps = new recSample();
        ps->data = new uint8_t[size];
        buf[i] = ps;
    }
    buffer_o = buf;
    bufSize = size;

}

bool AudioRecorder::Stop(void) {
    // in case already recording, stop recording and clear buffer queue
    SLuint32 curState;

    SLresult result = (*recItf_o)->GetRecordState(recItf_o, &curState);

    if (curState == SL_RECORDSTATE_STOPPED) {
        return SL_BOOLEAN_TRUE;
    }
    result = (*recItf_o)->SetRecordState(recItf_o, SL_RECORDSTATE_STOPPED);

    result = (*recBufQueueItf)->Clear(recBufQueueItf);


    return true;
}

AudioRecorder::~AudioRecorder() {
    // destroy audio recorder object, and invalidate all associated interfaces
    if (recObjectItf != nullptr) {
        (*recObjectItf)->Destroy(recObjectItf);
    }

}

void AudioRecorder::RegisterCallback(JNI_AUDIO_CALLBACK cb, void *ctx) {
    callback_f = cb;
}

