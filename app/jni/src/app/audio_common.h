//
// Created by ajl on 2021/9/6.
//

#ifndef HGHUAGERTP_APP_AUDIO_COMMON_H

#define HGHUAGERTP_APP_AUDIO_COMMON_H
#include <stdint.h>
#include "common.h"
#define EV_BUF_DATA_PTR(data) ((uint8_t **) (data))
#define EV_BUF_SIZE_PTR(data) ((uint32_t *) ((data) + sizeof(uint8_t *)))
#define EV_BUF_PTS_PTR(data) ((uint32_t *) ((data) + sizeof(uint8_t *)+sizeof(uint32_t)))
#define EV_BUF_PTR_LENGTH (sizeof(uint8_t *)+sizeof(uint32_t)+sizeof(uint32_t))



typedef struct {
    uint8_t *data;
    uint32_t size;
    uint16_t pts;
}ev_audio_t;
class AudioCommon{
public:
    static int InitEvent(uint8_t *data,ev_audio_t *he);
    static int ParseEvent(uint8_t *data, ev_audio_t *he);
};

#endif //HGHUAGERTP_APP_AUDIO_COMMON_H
