#include "ffplay.h"
#include <EGL/egl.h>
#include <android/native_window.h>
#include <malloc.h>

char vertexShaderSource[] =
        "#version 300 es                         \n"
        "layout(location = 0) in vec4 vPosition; \n"
        "void main()                             \n"
        "{                                       \n"
        "   gl_Position = vPosition;             \n"
        "}                                       \n";

char fragShaderSource[] =
        "#version 300 es                             \n"
        "precision mediump float;                    \n"
        "out vec4 fragColor;                         \n"
        "void main()                                 \n"
        "{                                           \n"
        "   fragColor = vec4 (1.0, 0.0, 0.0, 1.0);   \n"
        "}                                           \n";

EGLDisplay display;
EGLSurface surface;


/**
 * 把 shader info log 打印出来
 */
void shaderInfoLog(GLuint shader) {
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        GLchar *info = malloc(sizeof(GLchar) * length);
        glGetShaderInfoLog(shader, length, NULL, info);
        LOG("%s", info);
        free(info);
    }
}

/**
 * 把 program info log 打印出来
 */
void programInfoLog(GLuint program) {
    GLint length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        GLchar *info = malloc(sizeof(GLchar) * length);
        glGetProgramInfoLog(program, length, NULL, info);
        LOG("%s", info);
        free(info);
    }
}

EGLBoolean makeCurrent(NativeWindowType window) {
    EGLContext context;
    EGLint major;
    EGLint minor;
    EGLConfig config;
    EGLint numConfig;
    EGLint nativeVisualId;
    int ret;
    static const EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
            EGL_BLUE_SIZE,          8,
            EGL_GREEN_SIZE,         8,
            EGL_RED_SIZE,           8,
            EGL_NONE
    };
    static const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        LOG("eglGetDisplay fail %d", eglGetError());
        return EGL_FALSE;
    }

    if (eglInitialize(display, &major, &minor) == EGL_FALSE) {
        LOG("eglInitialize fail %d", eglGetError());
        eglTerminate(display);
        return EGL_FALSE;
    }
    LOG("egl version %d.%d", major, minor);

    if (eglChooseConfig(display, configAttribs, &config, 1, &numConfig) == EGL_FALSE) {
        LOG("eglChooseConfig fail %d", eglGetError());
        eglTerminate(display);
        return EGL_FALSE;
    }

    // 在 android 平台上需要做以下配置
    if (eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &nativeVisualId) == EGL_FALSE) {
        LOG("eglGetConfigAttrib EGL_NATIVE_VISUAL_ID fail %d", eglGetError());
        eglTerminate(display);
        return EGL_FALSE;
    }
    int32_t width = ANativeWindow_getWidth(window);
    int32_t height = ANativeWindow_getHeight(window);
    int32_t format = ANativeWindow_getFormat(window);
    LOG("native window, %dx%d, format:%d, nativeVisualId:%d", width, height, format, nativeVisualId);

    ret = ANativeWindow_setBuffersGeometry(window, width, height, nativeVisualId);
    if (ret) {
        LOG("ANativeWindow_setBuffersGeometry fail %d", ret);
        eglTerminate(display);
        return EGL_FALSE;
    }

    surface = eglCreateWindowSurface(display, config, window, NULL);
    if (surface == EGL_NO_SURFACE) {
        LOG("eglCreateWindowSurface fail %d", eglGetError());
        eglTerminate(display);
        return EGL_FALSE;
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        LOG("eglCreateContext fail %d", eglGetError());
        eglDestroySurface(display, surface);
        eglTerminate(display);
        return EGL_FALSE;
    }

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOG("eglMakeCurrent fail %d", eglGetError());
        eglDestroySurface(display, surface);
        eglDestroyContext(display, context);
        eglTerminate(display);
        return EGL_FALSE;
    }

    LOG("eglMakeCurrent success");
    return EGL_TRUE;
}

/**
 * 创建、加载和编译 shader
 * @param type GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
 * @return shader
 */
int loadShader(GLenum type, char *source, GLuint *outShader) {
    GLint compiled;
    GLuint shader;

    shader = glCreateShader(type);
    if (!shader) {
        LOG("glCreateShader fail");
        return -1;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        shaderInfoLog(shader);
        glDeleteShader(shader);
        return -1;
    }

    *outShader = shader;
    return 0;
}



int draw(int width, int height) {
    GLuint vertexShader;
    GLuint fragShader;
    GLuint program;
    GLint programLinked;
    GLfloat vertexes[] = {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
    };

    loadShader(GL_VERTEX_SHADER, vertexShaderSource, &vertexShader);
    if (!vertexShader) {
        LOG("load vertex shader fail");
        return -1;
    }
    loadShader(GL_FRAGMENT_SHADER, fragShaderSource, &fragShader);
    if (!fragShader) {
        LOG("load fragment shader fail");
        return -1;
    }

    program = glCreateProgram();
    if (!program) {
        LOG("glCreateProgram fail");
        return -1;
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &programLinked);
    if (!programLinked) {
        programInfoLog(program);
        glDeleteProgram(program);
        return -1;
    }

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glUseProgram(program);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertexes);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (eglSwapBuffers(display, surface) == EGL_FALSE){
        LOG("eglSwapBuffers fail %d", eglGetError());
        return -1;
    }
    return 0;
}