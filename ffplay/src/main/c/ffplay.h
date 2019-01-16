
#ifndef FFMPEG_ANDROID_EXAMPLE_FFPLAY_H
#define FFMPEG_ANDROID_EXAMPLE_FFPLAY_H


#include <android/log.h>
#include <jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <ctype.h>
#include <stdbool.h>
#include "libavformat/avformat.h"
#include <android/native_window.h>


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
void setNativeWindow(GLContext *ptr, ANativeWindow *window);

/**
 * 绘制前的准备：获取默认 display 的 read frame buffer & draw frame buffer 的控制权
 * it will destory context if false
 */
bool makeCurrent(GLContext *);

/**
 * 简单地绘制一个三角形
 */
bool drawTriangle(GLContext *);

/**
 * 绘制简单纹理的例子
 */
void drawSimpleTexture(GLContext *);


/************************** 使用 native window 来绘制帧 *******************************/

/**
 * 准备 native render
 */
bool prepareNativeRender(GLContext *ctx, AVCodecContext *cc);

/**
 * native render
 */
bool renderNativeWindow(AVFrame *frame, GLContext *ctx);


#endif //FFMPEG_ANDROID_EXAMPLE_FFPLAY_H