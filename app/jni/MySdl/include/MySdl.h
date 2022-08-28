//
// Created by ajl on 2021/6/8.
//

#ifndef ANDROID_PROJECT_MYSDL_H
#define ANDROID_PROJECT_MYSDL_H
#define MYSDL_DLL_EXPORT __attribute__((visibility("default")))
#include <cstdint>
#include "ex_hgmain.h"
#include "../android/audio_common.h"
void writeBufToPlay(uint8_t *data,int size,int pts);


MYSDL_DLL_EXPORT void myApiCreate(JNI_AUDIO_CALLBACK reccb,JNI_AUDIO_CALLBACK placb) ;

#endif //ANDROID_PROJECT_MYSDL_H
