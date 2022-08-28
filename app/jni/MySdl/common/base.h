//
// Created by ajl on 2021/6/16.
//

#ifndef HUAGERTP_APP_BASE_H

#define HUAGERTP_APP_BASE_H
#include <semaphore.h>
#include <android/log.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#ifdef __cplusplus
extern "C"{
#endif
void my__android_log_print(uint32_t level, uint32_t tag,const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#define ALOG(level, f1, ...)    my__android_log_print(level, f1, __VA_ARGS__)

#define ALOGV(f1,...)  ALOG(ANDROID_LOG_VERBOSE,f1,  __VA_ARGS__)
#define ALOGD(f1,...)  ALOG(ANDROID_LOG_DEBUG,f1,  __VA_ARGS__)
#define ALOGI(f1,...)  ALOG(ANDROID_LOG_INFO,f1,  __VA_ARGS__)
#define ALOGW(f1,...)  ALOG(ANDROID_LOG_WARN,f1,  __VA_ARGS__)
#define ALOGE(f1,...)  ALOG(ANDROID_LOG_ERROR,f1,  __VA_ARGS__)

#ifdef __cplusplus
extern "C"{
#endif
void toolsIntBigEndianV(uint8_t *d, uint8_t *val, uint32_t digits);
void toolsIntBigEndian(uint8_t *d, uint32_t val, uint32_t digits);
void toolsIntLitEndian(uint8_t *d, uint32_t val, uint32_t digits);
uint64_t hgetSysTimeMicros(void);
int uint32Sub(uint32_t newd,uint32_t oldd);
uint32_t uint32Add(uint32_t add1,uint32_t add2);
#ifdef __cplusplus
}
#endif
#endif //HUAGERTP_APP_BASE_H
