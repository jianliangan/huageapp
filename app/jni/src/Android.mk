LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

#MYSDL_PATH := ../MySdl
#FDK_AAC_PATH :=../fdk-aac-2.0.1
#OPENH264_PATH :=../openh264
#FFMPEG_PATH :=../ffmpeg
#X264_PATH :=../x264
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(FDK_AAC_PATH)/libAACdec/include \
#$(LOCAL_PATH)/$(FDK_AAC_PATH)/libAACenc/include \
#$(LOCAL_PATH)/$(FDK_AAC_PATH)/libSYS/include
LOCAL_C_INCLUDES := #$(LOCAL_PATH)/$(OPENH264_PATH)/include/api/svc \
#$(LOCAL_PATH)/$(OPENH264_PATH)/include/console/dec/inc \
#$(LOCAL_PATH)/$(OPENH264_PATH)/include/console/common/inc \
#$(LOCAL_PATH)/$(OPENH264_PATH)/include/encoder/core/inc \
#$(LOCAL_PATH)/$(OPENH264_PATH)/include/processing/interface \
#$(LOCAL_PATH)/$(OPENH264_PATH)/include/common/inc \
#$(LOCAL_PATH)/$(FFMPEG_PATH)/include/$(TARGET_ARCH_ABI) \
#$(LOCAL_PATH)/$(X264_PATH)/$(TARGET_ARCH_ABI)/include



# Add your application source files here...


ifeq ($(FOUND_CPP), "")
$(error error is "dfdfdfdff")
endif
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
$(wildcard $(LOCAL_PATH)/main.cpp) \
$(wildcard $(LOCAL_PATH)/app/*.cpp) \
$(wildcard $(LOCAL_PATH)/base/*.cpp) \
$(wildcard $(LOCAL_PATH)/net/protocol/*.cpp) \
$(wildcard $(LOCAL_PATH)/net/protocol/core/*.c) \
$(wildcard $(LOCAL_PATH)/net/protocol/core/*.cpp) \
$(wildcard $(LOCAL_PATH)/net/threads/*.cpp) \
$(wildcard $(LOCAL_PATH)/net/common/*.cpp) \
$(wildcard $(LOCAL_PATH)/net/*.cpp) \
$(wildcard $(LOCAL_PATH)/h264parse/*.c) \
$(wildcard $(LOCAL_PATH)/base/*.c))

    #APP_STL := c++_shared
   # NDK_TOOLCHAIN_VERSION := clang3.6
    APP_CPPFLAGS += -frtti -fexceptions

LOCAL_CPPFLAGS += -std=c++11 -Werror -D__this_android__
LOCAL_CFLAGS += -Werror  -D__this_android__
#LOCAL_SHARED_LIBRARIES := MySdl
#LOCAL_SHARED_LIBRARIES += fdk-aac
#LOCAL_SHARED_LIBRARIES += openh264
#LOCAL_SHARED_LIBRARIES += avcodec
#LOCAL_SHARED_LIBRARIES += avutil

#LOCAL_STATIC_LIBRARIES :=x264

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog  -ldl -landroid
# -ldl -landroid
include $(BUILD_SHARED_LIBRARY)
