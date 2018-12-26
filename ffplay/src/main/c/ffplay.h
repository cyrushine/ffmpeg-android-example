//
// Created by 林威 on 2018/12/25.
//

#ifndef FFMPEG_ANDROID_EXAMPLE_FFPLAY_H
#define FFMPEG_ANDROID_EXAMPLE_FFPLAY_H

#include <android/log.h>
#include <jni.h>

#define LOG_TAG "ffplay_native"
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern jmethodID jniGetFilePath;
extern jmethodID jniGetSurface;

JNIEXPORT void JNICALL testLog(JNIEnv *, jobject, jstring);
JNIEXPORT void JNICALL start(JNIEnv *, jobject);

#endif //FFMPEG_ANDROID_EXAMPLE_FFPLAY_H