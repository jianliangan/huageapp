//
// Created by ajl on 2021/6/8.
//

#include "native_setupJni.h"
#include "../common/base.h"
#include "stdlib.h"
#include "stdio.h"
#include "../test/func_test.h"
#include "../common/floatdata.h"
#include "ex_hgmain.h"
#include "../../src/ex_hgmain.h"
#include "speex/speex_preprocess.h"
#include <speex/speex_echo.h>

#define MYSDL_JAVA_PREFIX                                 org_libhuagertp_app
#define CONCAT1(prefix, class, function)                CONCAT2(prefix, class, function)
#define CONCAT2(prefix, class, function)                Java_ ## prefix ## _ ## class ## _ ## function
#define MYSDL_JAVA_INTERFACE(function)                    CONCAT1(MYSDL_JAVA_PREFIX, MySdl, function)

#define MYSDL_arraysize(array)    (sizeof(array)/sizeof(array[0]))
#define MYSDL_TABLESIZE(table)    MYSDL_arraysize(table)

static JavaVM *mJavaVM = NULL;
static pthread_key_t mThreadKey;
static jclass mMySdlClass;

static jmethodID midNotifyMyTest;
static jmethodID midMySdlNotifyReqPermission;
static jmethodID midMySdlNotifyPermission;

static jmethodID midMySdlNotifySendMessage;
static jmethodID midMySdlNotifyVideoData;

static int mfidFlagAudio;
static int mfidFlagCamera;

//static jobjectArray jRecbuffer_o;
JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeMainThRun)(JNIEnv
                                      *env,
                                      jclass cls
);

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeSendVideEncode)(JNIEnv
                                           *env,
                                           jclass cls, jobject bytebuf, jint size, jint timestamp,
                                           jint channel);

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativePushAdioDataPlay)(JNIEnv
                                             *env,
                                             jclass cls, jobject bytebuf, jint size
);

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeSetupMysdl)(JNIEnv
                                       *env,
                                       jclass cls
);

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeVirtualMain0)(JNIEnv
                                         *env,
                                         jclass cls
);

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeVirtualMain1)(JNIEnv
                                         *env,
                                         jclass cls
);

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeTest)(JNIEnv
                                 *env,
                                 jclass cls
);

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeCreateEnterRoom)(JNIEnv
                                            *env,
                                            jclass cls,
                                            jint roomid
);

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeLogin)(JNIEnv
                                  *env,
                                  jclass cls,
                                  jobjectArray params
);

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeStartStream)(JNIEnv
                                        *env,
                                        jclass cls

);

static void register_methods(JNIEnv *env,
                             const char *classname, JNINativeMethod
                             *methods,
                             int nb
);


static void
JNI_ThreadDestroyed(void *value) {
    /* The thread is being destroyed, detach it from the Java VM and set the mThreadKey value to NULL as required */
    JNIEnv *env = (JNIEnv *) value;
    if (env != NULL) {
        mJavaVM->DetachCurrentThread();
        pthread_setspecific(mThreadKey, nullptr);
        //assert(status == 0);
    }
}

static JNIEnv *getEnv() {
    JNIEnv *env = (JNIEnv *) pthread_getspecific(mThreadKey);
    if (env == nullptr) {

        int status = mJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);

        if (status == JNI_EDETACHED || env == NULL) {
            status = mJavaVM->AttachCurrentThread(&env, NULL);
            assert(status == 0);
        }
        status = pthread_setspecific(mThreadKey, env);
        assert(status == 0);
    }
    return env;
}

static void onRecvVideoJni(int type, hg_chain_node *hcn, int size,int pts) {
    JNIEnv *env = getEnv();
    jbyteArray byte_arr = env->NewByteArray(size);

    hg_Buf_Gen_t *hbgt;

    int total = 0;
    //ALOGI(0, "222222222222222222222222 size %d",
    //      size);

    while (hcn != nullptr) {
        hbgt = (hg_Buf_Gen_t *) hcn->data;
     //   ALOGI(0, "222222222222222222222222 size total %d len %d start %d",
     //         total, hbgt->len, hbgt->start);
        env->SetByteArrayRegion(byte_arr, total, hbgt->len, (jbyte *) hbgt->data + hbgt->start);
        total += hbgt->len;

        hcn = hcn->next;
    }
    ALOGI(0, "222222222222222222222222  ------------ total %d pts %d",
          total,pts);

    env->CallStaticVoidMethod(mMySdlClass, midMySdlNotifyVideoData, byte_arr, total,pts);
}

static void onRecvAudioJni(int type, uint8_t *data, int size,int pts) {
    writeBufToPlay((uint8_t *) data, size,pts);
}

static void onLoginJni() {
    JNIEnv *env = getEnv();
    jint arg1 = 1;
    jobject message = nullptr;
    env->CallStaticBooleanMethod(mMySdlClass, midMySdlNotifySendMessage, arg1, message);
}

/*
static int8_t MuLaw_Encode(int16_t number)
{
    const uint16_t MULAW_MAX = 0x1FFF;
    const uint16_t MULAW_BIAS = 33;
    uint16_t mask = 0x1000;
    uint8_t sign = 0;
    uint8_t position = 12;
    uint8_t lsb = 0;
    if (number < 0)
    {
        number = -number;
        sign = 0x80;
    }
    number += MULAW_BIAS;
    if (number > MULAW_MAX)
    {
        number = MULAW_MAX;
    }
    for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--)
        ;
    lsb = (number >> (position - 4)) & 0x0f;
    return (~(sign | ((position - 5) << 4) | lsb));
}

static int16_t MuLaw_Decode(int8_t number)
{
    const uint16_t MULAW_BIAS = 33;
    uint8_t sign = 0, position = 0;
    int16_t decoded = 0;
    number = ~number;
    if (number & 0x80)
    {
        number &= ~(1 << 7);
        sign = -1;
    }
    position = ((number & 0xF0) >> 4) + 5;
    decoded = ((1 << position) | ((number & 0x0F) << (position - 4))
               | (1 << (position - 5))) - MULAW_BIAS;
    return (sign == 0) ? (decoded) : (-(decoded));
}*/

#define numberf 3
//static FILE *files[numberf] = {0};

//static int tot = 0;

#define SPEEXSAMPLERATE 8000
#define SPEEXFRAMELEN 160
#define SPEEXFILTERLEN 800
//static SpeexEchoState *echo_state = nullptr;
//static SpeexPreprocessState *preprocess_state;

//only suitable <1
//void dectobin(float dec,int *bin){
//不用转了，int[3],int[2],int[1],int[0]
// 符号位  8位指数，后面就是二进制科学计数法的小数部分，自带进位功能，
// 1 1111111 1  11111。。。
//从2字节的第7位往后都是二进制小数点后面的
//}
//}



static void recDataCB(ev_audio_t *eat) {
   // func_test ftest;
   // ftest.set_test();
    //ftest.copy_test();
    //ftest.add_test();

  //  ftest.product_test();
    //ftest.sub_test();
//return;

   // int depth = 16;
   // int framenum = 160;//2chan
   // int framelen = framenum * (depth / 8);//16bit,24bit
  //  if (echo_state == nullptr) {
/*
        preprocess_state = speex_preprocess_state_init(SPEEXFRAMELEN,
                                                       SPEEXSAMPLERATE);

        echo_state = speex_echo_state_init(framenum, SPEEXFILTERLEN);
        int sampleRate = SPEEXSAMPLERATE;
        speex_echo_ctl(echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state);

        // 噪音去除

        int i = 0;
        float f = 0;
        i = 1;
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_DENOISE, &i);
        i = 2;
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_AGC, &i);
        i = -60;
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &i);
        i = 8000;
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_AGC_LEVEL, &i);
        i = 2;
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_DEREVERB, &i);
        f = .0;
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
        f = .0;
        speex_preprocess_ctl(preprocess_state, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
*/
        /*jieshu*/
        //
  //  }


//ajlatest
/*
    char *pp ;

    pp=new char[eat->size];
    memcpy(pp,eat->data,eat->size);
    int16_t *cur=(int16_t *)pp;
    for(int i=0;i<eat->size/2;i++){
        *cur=MuLaw_Decode(MuLaw_Encode(*cur));
        cur+=1;
    }



    int16_t *last=nullptr ;
    pp=new char[eat->size/2];
    memcpy(pp,eat->data,eat->size/2);
    int8_t *cur=(int8_t *)pp;
    last=(int16_t *)eat->data;
    for(int i=0;i<eat->size/2;i++){
        *cur=MuLaw_Encode(*last);
        cur+=1;
        last=last+1;
    }



    writeBufToPlay((unsigned char *)pp, eat->size);
    delete pp;
    return;*/
    int16_t *cur= (int16_t *)eat->data;
    int16_t *curs=cur;
    int size=eat->size/2;
    int offset=0;
    while(true){
        *(cur++)=(*(curs+offset)+*(curs+offset+1))/2;
        offset+=2;
        if(offset==size){
            break;
        }
    }
    eat->size=eat->size/2;

//ALOGI(0,"tytytytytytyt sizef %d pts %d",eat->size,eat->pts);


   // writeBufToPlay(eat->data, eat->size);
   // return;
    appAudioFramtoEnco(eat);
}

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeMainThRun)(JNIEnv
                                      *env,
                                      jclass cls
) {
    appMainThRun();
}

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeSendVideEncode)(JNIEnv
                                           *env,
                                           jclass cls, jobject bytebuf, jint size, jint timestamp,
                                           jint channel) {
    void *data = env->GetDirectBufferAddress(bytebuf);
    appSendVideo((uint8_t *) data, size, timestamp, channel);
}

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativePushAdioDataPlay)(JNIEnv
                                             *env,
                                             jclass cls, jobject bytebuf, jint size
) {
    void *data = env->GetDirectBufferAddress(bytebuf);
    writeBufToPlay((uint8_t *) data, size,0);
}

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeSetupMysdl)(JNIEnv
                                       *env,
                                       jclass cls
) {
//mmySdlClass = (jclass)(env->NewGlobalRef( cls));
// mMySdlClass = env->GetObjectClass( cls);
    mMySdlClass = (jclass) (env->NewGlobalRef(cls));
//mMySdlClass = env->FindClass("org/libhuagertp/app/MySdl");
// jfieldID fid = env->GetFieldID ( mMySdlClass, "aaa", "I");
//  jstring name = env->CallObjectMethod( job, mid_getName);
//  const char *name1 = env->GetStringUTFChars( name, NULL);

    pthread_key_create(&mThreadKey, JNI_ThreadDestroyed);


    midMySdlNotifyReqPermission = env->GetStaticMethodID(mMySdlClass, "NotifyReqPermission",
                                                         "(Ljava/lang/String;I)V");
    midMySdlNotifyPermission = env->GetStaticMethodID(mMySdlClass, "NotifyPermission", "(I)Z");

    midNotifyMyTest = env->GetStaticMethodID(mMySdlClass, "NotifyMyTest", "()V");
    midMySdlNotifyVideoData = env->GetStaticMethodID(mMySdlClass, "NotifyVideoData",
                                                     "([BII)V");
    midMySdlNotifySendMessage = env->GetStaticMethodID(mMySdlClass, "NotifySendMessage",
                                                       "(ILjava/lang/Object;)Z");


//midClipboardHasText = env->GetStaticMethodID( mmySdlClass, "clipboardHasText", "()Z");
/*
 *
jclass cls = env->GetObjectClass(obj);
//2.
jfieldID jfid = env->GetStaticFieldID(cls, "age", "I");
//3.
jint jage = env->GetStaticIntField(cls, jfid);
jage ++;
//4.
env->SetStaticIntField(cls,jfid, jage);
 */
    jfieldID fidAudio = env->GetStaticFieldID(mMySdlClass, "MY_PERMISSIONS_RECORD_AUDIO", "I");

    jfieldID fidCamera = env->GetStaticFieldID(mMySdlClass, "MY_PERMISSIONS_RECORD_CAMERA", "I");
    mfidFlagAudio = env->GetStaticIntField(mMySdlClass, fidAudio);
    mfidFlagCamera = env->GetStaticIntField(mMySdlClass, fidCamera);
    if (!midMySdlNotifyReqPermission || !midMySdlNotifyPermission || !midNotifyMyTest ||
        !midMySdlNotifySendMessage || !midMySdlNotifyVideoData) {
        __android_log_print(ANDROID_LOG_WARN,
                            "SDL",
                            "Missing some Java callbacks, do you have the latest version of SDLActivity.java?");
    }
    appInitProperty(onLoginJni, onRecvVideoJni, onRecvAudioJni);
    return 0;
}

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeTest)(JNIEnv
                                 *env,
                                 jclass cls
) {
    return;
}

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeCreateEnterRoom)(JNIEnv
                                            *env,
                                            jclass cls,
                                            int roomid
) {
    return appCreateEnterRoom(roomid);
}

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeLogin)(JNIEnv
                                  *env,
                                  jclass cls,
                                  jobjectArray params
) {

    int len = env->GetArrayLength(params);
    myjchar argv[len];
    for (int i = 0; i < len; i++) {
        const char *utf;
        char *arg = NULL;
        jstring string = (jstring) env->GetObjectArrayElement(params, i);
        jsize strlen = env->GetStringUTFLength(string);
        int lenstr = 2 * strlen + 1;
        arg = new char[lenstr];
        memset(arg, 0, lenstr);
        if (string) {
            utf = env->GetStringUTFChars(string, 0);
            if (utf) {
                memcpy(arg, utf, 2 * strlen);
                env->ReleaseStringUTFChars(string, utf);
            }
            env->DeleteLocalRef(string);
        }
        argv[i].data = arg;
        argv[i].size = lenstr;
    }
    appLogin(len, argv);//
    for (int i = 0; i < len; i++) {
        delete (argv[i].data);
    }

    return;
}

JNIEXPORT void JNICALL
MYSDL_JAVA_INTERFACE(nativeStartStream)(JNIEnv
                                        *env,
                                        jclass cls
) {

    audio_config_t recconfig;
    audio_config_t playconfig;
    recconfig.channels = CONF_STREAM_CAPTURE_CHANNELS;
    recconfig.deep = CONF_CAPTURE_DEEP;
    recconfig.samplesrate = CONF_CAPTURE_SAMPLESRATE;
    recconfig.frameperbuf = CONF_CAPTURE_FRAMEPERBUF;

    playconfig.channels = CONF_STREAM_PLAY_CHANNELS;
    playconfig.deep = CONF_PLAY_DEEP;
    playconfig.samplesrate = CONF_PLAY_SAMPLESRATE;
    playconfig.frameperbuf = CONF_PLAY_FRAMEPERBUF;
    appInitStream(&recconfig, &playconfig);
    return;
}

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeVirtualMain0)(JNIEnv
                                         *env,
                                         jclass
                                         cls
) {
/*
int64_t t0=hgetSysTimeMicros()/1000;
for(int i=0;i<100000;i++){
    env->CallStaticVoidMethod(mMySdlClass, midNotifyMyTest);
}
int64_t t1=hgetSysTimeMicros()/1000;
FILE *f2;
f2=fopen("/data/data/org.libhuagertp.app/logcc.log","a");
char out[4096];
sprintf(out,"tttttttttttttttttttt %d",int(t1-t0));
fprintf(f2,"%s",out);

fclose(f2);*/


    jstring param = env->NewStringUTF("android.permission.RECORD_AUDIO");


    env->
            CallStaticVoidMethod(mMySdlClass, midMySdlNotifyReqPermission, param, mfidFlagAudio
    );
    int re = env->CallStaticBooleanMethod(mMySdlClass, midMySdlNotifyPermission, mfidFlagAudio);
    if (re == 0) {
        return -1;
    }

    myApiCreate(recDataCB, nullptr);
    return 0;
}

JNIEXPORT int JNICALL
MYSDL_JAVA_INTERFACE(nativeVirtualMain1)(JNIEnv
                                         *env,
                                         jclass
                                         cls
) {

    return 0;
}


//under is a block in initialization
static JNINativeMethod MySdl_tab[] = {
        {"nativeMainThRun",        "()V",                         (void *) MYSDL_JAVA_INTERFACE(
                nativeMainThRun)},
        {"nativeSendVideEncode",   "(Ljava/nio/ByteBuffer;III)V", (void *) MYSDL_JAVA_INTERFACE(
                nativeSendVideEncode)},
        {"nativePushAdioDataPlay", "(Ljava/nio/ByteBuffer;I)V",   (void *) MYSDL_JAVA_INTERFACE(
                nativePushAdioDataPlay)},
        {"nativeSetupMysdl",       "()I",                         (void *) MYSDL_JAVA_INTERFACE(
                nativeSetupMysdl)},
        {"nativeVirtualMain0",     "()I",                         (void *) MYSDL_JAVA_INTERFACE(
                nativeVirtualMain0)},
        {"nativeVirtualMain1",     "()I",                         (void *) MYSDL_JAVA_INTERFACE(
                nativeVirtualMain1)},
        {"nativeTest",             "()V",                         (void *) MYSDL_JAVA_INTERFACE(
                nativeTest)},
        {"nativeStartStream",      "()V",                         (void *) MYSDL_JAVA_INTERFACE(
                nativeStartStream)},
        {"nativeLogin",            "([Ljava/lang/String;)V",      (void *) MYSDL_JAVA_INTERFACE(
                nativeLogin)},
        {"nativeCreateEnterRoom",  "(I)I",                        (void *) MYSDL_JAVA_INTERFACE(
                nativeCreateEnterRoom)}
};

static void
register_methods(JNIEnv *env,
                 const char *classname, JNINativeMethod
                 *methods,
                 int nb
) {
    jclass clazz = env->FindClass(classname);
    if (clazz == NULL || env->
            RegisterNatives(clazz, methods, nb
    ) < 0) {
        __android_log_print(ANDROID_LOG_ERROR,
                            "MySdl", "Failed to register methods of %s",
                            classname);
        return;
    }
}

JNIEXPORT jint

JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    mJavaVM = vm;
    JNIEnv *env = NULL;

    if (mJavaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "SDL", "Failed to get JNI Env");
        return JNI_VERSION_1_4;
    }

    register_methods(env, "org/libhuagertp/app/MySdl", MySdl_tab, MYSDL_arraysize(MySdl_tab));


    return JNI_VERSION_1_4;
}

