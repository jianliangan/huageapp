#ifndef _HUAGE_COMMON_HEAD_
#define _HUAGE_COMMON_HEAD_

enum AUDIO_codecs{
    AUDIO_G711=0,AUDIO_AAC
};

typedef struct audio_config_t{
    int channels;
    int deep;
    int samplesrate;
    int frameperbuf;
    AUDIO_codecs codec;
    void init(){
        channels=0;
        deep=0;
        samplesrate=0;
        frameperbuf=0;
        codec=AUDIO_G711;
    }
} audio_config_t;
#endif
