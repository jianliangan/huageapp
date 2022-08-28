//
// Created by ajl on 2021/1/18.
//

#include "audio_source.h"
#include "../net/protocol/core/tools.h"
#include "app.h"

AudioSource::AudioSource() {
    finished = true;
}

void AudioSource::Callback(void *udata, uint8_t *stream, uint32_t len) {

}

int AudioSource::CaptureStart() {
return 0;
}

void *
AudioSource::ConsumeQueue(void *ctx) {
return nullptr;
}

int AudioSource::Loop() {
    return 0;
}

void AudioSource::QueueOutAudio(uint8_t *data, uint32_t size) {

}