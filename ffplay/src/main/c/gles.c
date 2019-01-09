#include "ffplay.h"
#include <EGL/egl.h>
#include <android/native_window.h>
#include <malloc.h>

struct GLContext {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    int32_t winWidth;
    int32_t winHeight;
};

GLContext* createGLContext() {
    return malloc(sizeof(GLContext));
}

void destoryGLContext(GLContext *ptr) {
    if (ptr != NULL) {
        eglDestroySurface(ptr->display, ptr->surface);
        eglDestroyContext(ptr->display, ptr->context);
        eglTerminate(ptr->display);
        free(ptr);
    }
}


/**
 * 把 shader info log 打印出来
 */
static void shaderInfoLog(GLuint shader) {
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
static void programInfoLog(GLuint program) {
    GLint length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        GLchar *info = malloc(sizeof(GLchar) * length);
        glGetProgramInfoLog(program, length, NULL, info);
        LOG("%s", info);
        free(info);
    }
}

bool makeCurrent(EGLNativeWindowType window, GLContext *ctx) {
    if (ctx == NULL) {
        LOG("ctx can't be null");
        return false;
    }

    static const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };


    ctx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (ctx->display == EGL_NO_DISPLAY) {
        LOG("eglGetDisplay fail %d", eglGetError());
        destoryGLContext(ctx);
        return false;
    }


    EGLint major;
    EGLint minor;
    if (eglInitialize(ctx->display, &major, &minor) == EGL_FALSE) {
        LOG("eglInitialize fail %d", eglGetError());
        destoryGLContext(ctx);
        return false;
    }
    LOG("egl version %d.%d", major, minor);


    static const EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
            EGL_BLUE_SIZE,          8,
            EGL_GREEN_SIZE,         8,
            EGL_RED_SIZE,           8,
            EGL_NONE
    };
    EGLConfig config;
    EGLint numConfig;
    if (eglChooseConfig(ctx->display, configAttribs, &config, 1, &numConfig) == EGL_FALSE) {
        LOG("eglChooseConfig fail %d", eglGetError());
        destoryGLContext(ctx);
        return false;
    }


    // 在 android 平台上需要做以下配置
    EGLint nativeVisualId;
    if (eglGetConfigAttrib(ctx->display, config, EGL_NATIVE_VISUAL_ID, &nativeVisualId) == EGL_FALSE) {
        LOG("eglGetConfigAttrib EGL_NATIVE_VISUAL_ID fail %d", eglGetError());
        destoryGLContext(ctx);
        return false;
    }
    ctx->winWidth = ANativeWindow_getWidth(window);
    ctx->winHeight = ANativeWindow_getHeight(window);
    int32_t winFormat = ANativeWindow_getFormat(window);
    LOG("native window, %dx%d, format:%d, nativeVisualId:%d", ctx->winWidth, ctx->winHeight, winFormat, nativeVisualId);

    int ret = ANativeWindow_setBuffersGeometry(window, ctx->winWidth, ctx->winHeight, nativeVisualId);
    if (ret) {
        LOG("ANativeWindow_setBuffersGeometry fail %d", ret);
        destoryGLContext(ctx);
        return false;
    }


    ctx->surface = eglCreateWindowSurface(ctx->display, config, window, NULL);
    if (ctx->surface == EGL_NO_SURFACE) {
        LOG("eglCreateWindowSurface fail %d", eglGetError());
        destoryGLContext(ctx);
        return false;
    }


    ctx->context = eglCreateContext(ctx->display, config, EGL_NO_CONTEXT, contextAttribs);
    if (ctx->context == EGL_NO_CONTEXT) {
        LOG("eglCreateContext fail %d", eglGetError());
        destoryGLContext(ctx);
        return false;
    }


    if (eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context) == EGL_FALSE) {
        LOG("eglMakeCurrent fail %d", eglGetError());
        destoryGLContext(ctx);
        return false;
    }

    LOG("eglMakeCurrent success");
    return true;
}

/**
 * 创建、加载和编译 shader
 * @param type GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
 * @return shader
 */
static int loadShader(GLenum type, const char *source, GLuint *outShader) {
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


/**
 * 创建、链接 program，并加载对应的 vertex shader & fragment shader
 */
static GLuint loadProgram(const char *vertSource, const char *fragSource) {
    GLuint vertShader;
    GLuint fragShader;
    GLuint program;
    GLint programLinked;

    loadShader(GL_VERTEX_SHADER, vertSource, &vertShader);
    if (!vertShader) {
        LOG("load vertex shader fail");
        return 0;
    }
    loadShader(GL_FRAGMENT_SHADER, fragSource, &fragShader);
    if (!fragShader) {
        LOG("load fragment shader fail");
        return 0;
    }

    program = glCreateProgram();
    if (!program) {
        LOG("glCreateProgram fail");
        return 0;
    }

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &programLinked);
    if (!programLinked) {
        programInfoLog(program);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}


bool drawTriangle(GLContext *ctx) {

    static const char vertSource[] =
            "#version 300 es                         \n"
            "layout(location = 0) in vec4 vPosition; \n"
            "void main()                             \n"
            "{                                       \n"
            "   gl_Position = vPosition;             \n"
            "}                                       \n";

    // 指定像素值：红色
    static const char fragSource[] =
            "#version 300 es                             \n"
            "precision mediump float;                    \n"
            "out vec4 fragColor;                         \n"
            "void main()                                 \n"
            "{                                           \n"
            "   fragColor = vec4 (1.0, 0.0, 0.0, 1.0);   \n"
            "}                                           \n";

    GLuint program = loadProgram(vertSource, fragSource);
    if (!program) {
        LOG("glCreateProgram fail");
        return false;
    }


    glViewport(0, 0, ctx->winWidth, ctx->winHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glUseProgram(program);


    // 输入顶点坐标数据
    GLfloat vertexes[] = {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
    };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertexes);
    glEnableVertexAttribArray(0);


    glDrawArrays(GL_TRIANGLES, 0, 3);
    if (eglSwapBuffers(ctx->display, ctx->surface) == EGL_FALSE){
        LOG("eglSwapBuffers fail %d", eglGetError());
        return false;
    }
    return true;
}

void drawSimpleTexture(GLContext *ctx) {

    // 准备 program
    static const char vShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 a_position;   \n"
            "layout(location = 1) in vec2 a_texCoord;   \n"
            "out vec2 v_texCoord;                       \n"
            "void main()                                \n"
            "{                                          \n"
            "   gl_Position = a_position;               \n"
            "   v_texCoord = a_texCoord;                \n"
            "}                                          \n";

    static const char fShaderStr[] =
            "#version 300 es                                     \n"
            "precision mediump float;                            \n"
            "in vec2 v_texCoord;                                 \n"
            "layout(location = 0) out vec4 outColor;             \n"
            "uniform sampler2D s_texture;                        \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  outColor = texture( s_texture, v_texCoord );      \n"
            "}                                                   \n";

    GLuint program = loadProgram(vShaderStr, fShaderStr);
    if (!program) {
        LOG("loadProgram fail");
        return;
    }
    glViewport ( 0, 0, ctx->winWidth, ctx->winHeight );
    glClear ( GL_COLOR_BUFFER_BIT );
    glUseProgram ( program );


    // 输入顶点数据
    GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0
                            -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
    };

    // Load the vertex position
    glVertexAttribPointer ( 0, 3, GL_FLOAT,
                            GL_FALSE, 5 * sizeof ( GLfloat ), vVertices );
    // Load the texture coordinate
    glVertexAttribPointer ( 1, 2, GL_FLOAT,
                            GL_FALSE, 5 * sizeof ( GLfloat ), &vVertices[3] );

    glEnableVertexAttribArray ( 0 );
    glEnableVertexAttribArray ( 1 );


    // 上传纹理数据
    // 2x2 Image, 3 bytes per pixel (R, G, B)
    GLubyte pixels[4 * 3] = {
            255,   0,   0,  // Red
              0, 255,   0,  // Green
              0,   0, 255,  // Blue
            255, 255,   0   // Yellow
    };
    GLuint textureId;

    // Use tightly packed data
    // 「UNPACK」指解包，从内容读入 opengl；「PACK」指压包，指从 opengl 读入内存
    // 一个字节对齐，也即纹理数据是紧密地存储在一起的
    glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

    // Bind the texture
    // 可以这么想象：opengl 有 32 个纹理单元（texture unit），专门存储纹理
    // 这里设置当前活动的是「纹理单元 0」，那么后续对纹理的操作「glTex...」都是针对「纹理单元 0」的
    glActiveTexture ( GL_TEXTURE0 );

    // Generate a texture object
    // 可以想象一个纹理单元有很多的 field(target): GL_TEXTURE_2D（2D 纹理）, GL_TEXTURE_3D（3D 纹理）, GL_TEXTURE_CUBE_MAP（立方图）...
    // 而对纹理的操作「glTex...」不能用于 texture object，只能是当前活动纹理单元的某个 field(target)
    // 这里构造一个纹理对象 texture object，并绑定到当前活动纹理单元的某个 target 上；后续对这个 target 的操作就是对这个纹理对象的操作（那么这个纹理对象有什么用？目前没用到...）
    glGenTextures ( 1, &textureId );

    // Bind the texture object
    glBindTexture ( GL_TEXTURE_2D, textureId );

    // Load the texture
    // 上传纹理数据，这个是上传到当前活动纹理单元的某个 target 上
    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );

    // Set the filtering mode
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );


    // Set the sampler texture unit to 0
    // 还记得上面设置的活动纹理单元是「0」么？（而且把 2D 纹理数据上传到它的 GL_TEXTURE_2D 上了）
    // 这里设置全局只读 2D 取样器指向第「0」个纹理单元（你会发现这样的「设值」是很抽象的（0 竟然表示纹理单元 0），但有可以理解（取样器需要的是纹理单元而不是数值 0））
    GLint textureLoction = glGetUniformLocation ( program, "s_texture" );
    glUniform1i ( textureLoction, 0 );


    // 绘制两个三角形，形成矩形
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );


    // 输出到屏幕
    if (eglSwapBuffers(ctx->display, ctx->surface) == EGL_FALSE){
        LOG("eglSwapBuffers fail %d", eglGetError());
    }
}