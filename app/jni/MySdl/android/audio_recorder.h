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

#ifndef NATIVE_AUDIO_AUDIO_RECORDER_H
#define NATIVE_AUDIO_AUDIO_RECORDER_H

#include <sys/types.h>

#include "audio_common.h"
typedef struct recSample {
    uint8_t *data;
    recSample():data(nullptr){}
} recSample;
class AudioRecorder {

    int bufSize;

    SLObjectItf recObjectItf;
    SLRecordItf recItf_o;

    SLAndroidSimpleBufferQueueItf recBufQueueItf;
    recSample **buffer_o;

    JNI_AUDIO_CALLBACK callback_f;


public:

    std::atomic<uint32_t> produceCount;

    std::atomic<uint32_t> consumeCount;//It's actually under the control of the Java layer
    explicit AudioRecorder(SampleFormat *, SLEngineItf engineEngine);

    ~AudioRecorder();

    bool Start(void);

    bool Stop(void);

    void createBuffer(int len, int size);

    static void ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq, void *ctx);

    void RegisterCallback(JNI_AUDIO_CALLBACK cb, void *ctx);

};

#endif  // NATIVE_AUDIO_AUDIO_RECORDER_H
