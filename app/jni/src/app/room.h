//
// Created by ajl on 2021/9/14.
//

#ifndef HUAGERTP_APP_ROOM_H
#define HUAGERTP_APP_ROOM_H
#include <stdint.h>
typedef struct Room{
int id;
uint64_t sysms;
uint8_t *audioCache;
int audioCaLen;
} Room;
#endif //HUAGERTP_APP_ROOM_H
