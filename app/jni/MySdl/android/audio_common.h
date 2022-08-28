//
// Created by ajl on 2021/6/22.
//

#ifndef HUAGERTP_APP_AUDIO_COMMON_H
#define HUAGERTP_APP_AUDIO_COMMON_H
#include <cstring>
#include <cassert>
#include "../common/base.h"
#include "ex_hgaudio_common.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <atomic>

/*
 * Audio Sample Controls...
 */


/*
 * Sample Buffer Controls...
 */
//#define RECORD_DEVICE_KICKSTART_BUF_COUNT 2
//#define PLAY_KICKSTART_BUFFER_COUNT 3
#define CONF_STREAM_CAPTURE_CHANNELS  1
#define CONF_REND_CAPTURE_CHANNELS  2
#define CONF_CAPTURE_ANDROID_DATAFORMAT_PCM SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT
#define CONF_CAPTURE_DEEP 16
//#define CONF_CAPTURE_ANDROID_DATAFORMAT_PCM SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT
//#define CONF_CAPTURE_DEEP 8

#define CONF_CAPTURE_SAMPLESRATE   8000
#define CONF_CAPTURE_FRAMEPERBUF  160   //aac must be 1024,20ms simples count

#define CONF_STREAM_PLAY_CHANNELS  1
#define CONF_REND_PLAY_CHANNELS  2
#define CONF_PLAY_ANDROID_DATAFORMAT_PCM SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT
#define CONF_PLAY_DEEP 16
//#define CONF_PLAY_ANDROID_DATAFORMAT_PCM SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT
//#define CONF_PLAY_DEEP 8

#define CONF_PLAY_SAMPLESRATE   8000
#define CONF_PLAY_FRAMEPERBUF  160   //aac must be 1024,20ms simples count

#define DEVICE_RECODE_BUFFER_QUEUE_LEN 50//frames length
#define DEVICE_PLAY_BUFFER_QUEUE_LEN 50 //这个长度可以加大点，不会影响延迟，但是当网络不好的时候可以提高抗抖动，如果长度小，抖动时会有数据覆盖
//否则的话就要加缓冲，因为用的异步线程池，每个任务都不能长时间等待。会卡主其他任务




typedef struct SampleFormat {
    int channels;
    uint32_t sampleRate_;
    uint32_t representation_;  // android extensions
    SampleFormat():channels(0),sampleRate_(0),representation_(0){}
} SampleFormat;
extern void ConvertToSLSampleFormat(SLAndroidDataFormat_PCM_EX* pFormat,
                                    SampleFormat* format);

/*
 * GetSystemTicks(void):  return the time in micro sec
 */
__inline__ uint64_t GetSystemTicks(void) {
    struct timeval Time;
    gettimeofday(&Time, NULL);

    return (static_cast<uint64_t>(1000000) * Time.tv_sec + Time.tv_usec);
}

#define SLASSERT(x)                   \
  do {                                \
    assert(SL_RESULT_SUCCESS == (x)); \
    (void)(x);                        \
  } while (0)

/*
 * Interface for player and recorder to communicate with engine
 */
#define ENGINE_SERVICE_MSG_KICKSTART_PLAYER 1
#define ENGINE_SERVICE_MSG_RETRIEVE_DUMP_BUFS 2
#define ENGINE_SERVICE_MSG_RECORDED_AUDIO_AVAILABLE 3


typedef void (*JNI_AUDIO_CALLBACK)(ev_audio_t *eat);

#endif //HUAGERTP_APP_AUDIO_COMMON_H
