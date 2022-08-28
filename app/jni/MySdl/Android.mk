LOCAL_PATH := $(call my-dir)

###########################
#
# MySdl shared library
#
###########################

include $(CLEAR_VARS)
SPEEX_PATH := ../speexdsp-1.2rc3
LOCAL_MODULE := MySdl
SRC_PATH := ../src
LOCAL_SHARED_LIBRARIES += main
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \
$(LOCAL_PATH)/$(SRC_PATH) \
$(LOCAL_PATH)/$(SPEEX_PATH)/include


#LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/android/*.cpp) \
	$(wildcard $(LOCAL_PATH)/common/*.c) \
	$(wildcard $(LOCAL_PATH)/test/*.cpp) \
    $(wildcard $(LOCAL_PATH)/common/*.cpp))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_CFLAGS += \
	-Wall -Wextra \
	-Wmissing-prototypes \
	-Wunreachable-code-break \
	-Wunneeded-internal-declaration \
	-Wmissing-variable-declarations \
	-Wfloat-conversion \
	-Wshorten-64-to-32 \
	-Wunreachable-code-return \
	-Wshift-sign-overflow \
	-Wstrict-prototypes \
	-Wkeyword-macro


# Warnings we haven't fixed (yet)
LOCAL_CFLAGS += -Wno-unused-parameter -Wno-sign-compare -O3

LOCAL_CPPFLAGS += -std=c++11 -Werror -O3
LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid -lOpenSLES -O3

ifeq ($(NDK_DEBUG),1)
    cmd-strip :=
endif
LOCAL_STATIC_LIBRARIES :=SpeexDsp
LOCAL_STATIC_LIBRARIES += cpufeatures

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)

