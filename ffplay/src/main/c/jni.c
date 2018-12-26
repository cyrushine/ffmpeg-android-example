#include <jni.h>

#include "ffplay.h"

#define JAVA_JNI_CLASS "com/example/ffplay/FFplayJNI"
#define JNI_VERSION JNI_VERSION_1_6

typedef struct JMethodMapping {
    jmethodID *methodId;
    char *funcName;
    char *signature;
} JMethodMapping;

jmethodID jniGetFilePath;
jmethodID jniGetSurface;

static JMethodMapping jMethods[] = {
        {&jniGetFilePath, "getFilePath", "()Ljava/lang/String;"},
        {&jniGetSurface, "getSurface", "()Landroid/view/Surface;"}
};

static JNINativeMethod jniNativeMethods[] = {
        {"testLog", "(Ljava/lang/String;)V", testLog},
        {"start", "()V", start}
};

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint ret;
    int loop;
    jmethodID methodId = NULL;

    // 通用的代码块
    ret = (*vm)->GetEnv(vm, (void **) &env, JNI_VERSION);
    if (ret != JNI_OK) {
        LOG("jni onload GetEnv fail, jni version %d, error %d", JNI_VERSION, ret);
        return -1;
    }
    LOG("JNI_OnLoad");

    // 找到与 native 交互的 java class
    jclass jniClz = (*env)->FindClass(env, JAVA_JNI_CLASS);
    if (jniClz == NULL) {
        LOG("can't not find java class %s", JAVA_JNI_CLASS);
    }

    // 找到与 native 交互的 java function
    loop = sizeof(jMethods) / sizeof(JMethodMapping);
    for (int i = 0; i < loop; ++i) {
        methodId = (*env)->GetMethodID(env, jniClz, jMethods[i].funcName, jMethods[i].signature);
        if (methodId == NULL) {
            LOG("method %s not found", jMethods[i].funcName);
            return -1;
        }
        *jMethods[i].methodId = methodId;
        methodId = NULL;
    }

    // 注册 java native functions
    ret = (*env)->RegisterNatives(env, jniClz, jniNativeMethods, sizeof(jniNativeMethods) / sizeof(JNINativeMethod));
    if (ret) {
        LOG("RegisterNatives fail %d", ret);
        return -1;
    }

    return JNI_VERSION;
}