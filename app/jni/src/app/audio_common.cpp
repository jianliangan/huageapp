//
// Created by ajl on 2021/9/2.
//
#include "audio_common.h"
int AudioCommon::InitEvent(uint8_t *data,ev_audio_t *he) {
    *EV_BUF_SIZE_PTR(data) = he->size;
    *EV_BUF_DATA_PTR(data) = he->data;
    *EV_BUF_PTS_PTR(data) = he->pts;
    return 0;
}
int AudioCommon::ParseEvent(uint8_t *data, ev_audio_t *he) {

    he->size =*EV_BUF_SIZE_PTR(data) ;
    he->data=*EV_BUF_DATA_PTR(data);
    he->pts= *EV_BUF_PTS_PTR(data);

    return 0;
}
