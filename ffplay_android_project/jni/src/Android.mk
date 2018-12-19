LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../SDL/src/core/android

# Add your application source files here...
LOCAL_SRC_FILES := ffplay.c

LOCAL_SHARED_LIBRARIES := SDL2 avcodec avdevice avfilter avformat avutil swresample swscale

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)