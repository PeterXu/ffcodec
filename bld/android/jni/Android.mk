ROOT_PATH := $(call my-dir)/../../..
EXT_PATH := $(ROOT_PATH)/ffmpeg
LOCAL_PATH := $(ROOT_PATH)/src
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES:= \
	LogTrace.cpp \
	FFDecoder.cpp  \
	FFEncoder.cpp  \
	FFVideoParam.cpp \
	FFAudioParam.cpp 

LOCAL_SHARED_LIBRARIES := 
LOCAL_STATIC_LIBRARIES := 

LOCAL_LDLIBS := -llog -L../../libs/armeabi-v7a -lavutil -lavcodec -lavformat -lswscale

LOCAL_MODULE := libivyffmpeg

LOCAL_C_INCLUDES := 	\
	$(LOCAL_PATH)		\
	$(EXT_PATH) 

LOCAL_CFLAGS := -DANDROID

include $(BUILD_SHARED_LIBRARY)
