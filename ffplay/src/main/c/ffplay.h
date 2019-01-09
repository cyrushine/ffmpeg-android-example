//
// Created by 林威 on 2018/12/25.
//

#ifndef FFMPEG_ANDROID_EXAMPLE_FFPLAY_H
#define FFMPEG_ANDROID_EXAMPLE_FFPLAY_H

#include <android/log.h>
#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <ctype.h>
#include <stdbool.h>

#define LOG_TAG "ffplay_native"
#define LOG_AV_TAG "ffplay_av"
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern jmethodID jniGetFilePath;
extern jmethodID jniGetSurface;

JNIEXPORT void JNICALL testLog(JNIEnv *, jobject, jstring);
JNIEXPORT void JNICALL start(JNIEnv *, jobject);

// opengl es
struct GLContext;
typedef struct GLContext GLContext;

GLContext* createGLContext();

void destoryGLContext(GLContext *ptr);

/**
 * 绘制前的准备：获取默认 display 的 read frame buffer & draw frame buffer 的控制权
 * it will destory context if false
 */
bool makeCurrent(EGLNativeWindowType, GLContext *);

/**
 * 简单地绘制一个三角形
 */
bool drawTriangle(GLContext *);

/**
 * 绘制纹理的例子
 */
void drawSimpleTexture(GLContext *);



#endif //FFMPEG_ANDROID_EXAMPLE_FFPLAY_H