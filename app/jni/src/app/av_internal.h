//
// Created by ajl on 2021/4/26.
//

#ifndef ANDROID_PROJECT_VIDEOAUDIOINTERNAL_H
#define ANDROID_PROJECT_VIDEOAUDIOINTERNAL_H
#include "../net/protocol/core/tools.h"

typedef struct chainPicNode {
    int width;
    int height;
    int ystride;
    int uvstride;
    uint8_t *pic;
    void *opaque;
    uint8_t ref;
    chainPicNode *next;
} chainPicNode;
typedef struct chainPic {
    int ySize;
    int picSize;
    int ystride;
    int uvstride;
    int width;
    int height;
    chainPicNode *left;
    chainPicNode *right;
} chainPic;
#endif //ANDROID_PROJECT_INTERNAL_H