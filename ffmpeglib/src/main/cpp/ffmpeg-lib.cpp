#include <jni.h>
#include <string>
#include <pthread.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#define TAG "ffmpeg_lib"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)


/**
 * decode and render to window
 * by linwei
 * @param env
 * @param type
 * @param surface
 */
JNIEXPORT void JNICALL
Java_com_example_ffmpeglib_FFmpegLib_renderToWindowByMe(JNIEnv *env, jclass type, jobject surface) {
    av_register_all();

    // 打开视频文件
    const char *video_path = "/sdcard/video.mkv";
    AVFormatContext *format_ctx = avformat_alloc_context();
    if (avformat_open_input(&format_ctx, video_path, NULL, NULL) != 0) {
        LOGD("open file fail");
        return;
    }

    // 解封装，并找到合适的视频流
    if (avformat_find_stream_info(format_ctx, NULL) != 0) {
        LOGD("find stream info fail");
        return;
    }
    int stream_idx = av_find_best_stream(format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (stream_idx < 0) {
        LOGD("vaild stream not found");
        return;
    }
    AVStream *stream = format_ctx->streams[stream_idx];

    // 找到并打开解码器，创建解码上下文
    AVCodec *decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        LOGD("decoder not found");
        return;
    }
    AVCodecContext *codec_ctx = avcodec_alloc_context3(decoder);
    if (avcodec_parameters_to_context(codec_ctx, stream->codecpar) < 0) {
        LOGD("parameters to ctx fail");
        return;
    }
    if (avcodec_open2(codec_ctx, decoder, NULL) != 0) {
        LOGD("open decoder fail");
        return;
    }

    // 配置 android window buffer
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(window, codec_ctx->width, codec_ctx->height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;

    // 需要将 yuv -> rgba
    SwsContext *sws_ctx = sws_getContext(
            codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, NULL, NULL, NULL
    );
    // 这个 frame 用来存储 rgba pixel format frame，data buffer 需要自己分配和回收
    AVFrame *frame_rgba = av_frame_alloc();
    if (av_image_alloc(frame_rgba->data, frame_rgba->linesize, codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGBA, 1) < 0) {
        LOGD("alloc rgba frame fail");
        return;
    }

    // 循环处理视频文件里的每一帧
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    int ret = 0;
    while (av_read_frame(format_ctx, pkt) == 0) {
        if (pkt->stream_index == stream_idx) {

            // 解码
            ret = avcodec_send_packet(codec_ctx, pkt);
            if (ret != 0) {
                LOGD("send packet fail: %s", av_err2str(ret));
                break;
            }
            while (avcodec_receive_frame(codec_ctx, frame) == 0) {

                // yuv -> rgba
                sws_scale(sws_ctx,
                          (uint8_t const *const *) frame->data, frame->linesize, 0, codec_ctx->height,
                          frame_rgba->data, frame_rgba->linesize);
                frame_rgba->width = frame->width;
                frame_rgba->height = frame->height;

                // render to window
                if (ANativeWindow_lock(window, &window_buffer, NULL) != 0) {
                    LOGD("lock window fail");
                    break;
                }

                // 将 rgba frame data buffer copy to window buffer
                // rgba frame's strike == frame width, but window buffer's strike != frame width，所以需要逐行复制 picture line
                uint8_t *dst = (uint8_t *) window_buffer.bits;
                int dst_strike = window_buffer.stride * 4;
                uint8_t *src = frame_rgba->data[0];
                int src_strike = frame_rgba->linesize[0];
                for (int i = 0; i < frame_rgba->height; i++) {
                    memcpy(dst + i * dst_strike, src + i * src_strike, src_strike);
                }

                if (ANativeWindow_unlockAndPost(window) != 0) {
                    LOGD("post frame to window fail");
                    break;
                }
            }
        }
    }

    av_freep(frame_rgba->data[0]);
    av_frame_free(&frame);
    av_frame_free(&frame_rgba);
    av_packet_free(&pkt);
    sws_freeContext(sws_ctx);
    avcodec_free_context(&codec_ctx);
    avformat_close_input(&format_ctx);
}

/**
 * it show the flow of decode and render to window
 * @param env
 * @param type
 * @param surface
 */
JNIEXPORT void JNICALL
Java_com_example_ffmpeglib_FFmpegLib_renderToWindow(
        JNIEnv *env, jclass type, jobject surface) {
    const char *video_path = "/sdcard/video.mkv";

    av_register_all();

    AVFormatContext *pFormatCtx = avformat_alloc_context();

    // Open video file
    if (avformat_open_input(&pFormatCtx, video_path, NULL, NULL) != 0) {

        LOGD("Couldn't open file:%s\n", video_path);
        return; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGD("Couldn't find stream information.");
        return;
    }

    // Find the first video stream
    int videoStream = -1, i;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
            && videoStream < 0) {
            videoStream = i;
            LOGD("宽高：%d x %d", pFormatCtx->streams[i]->codecpar->width, pFormatCtx->streams[i]->codecpar->height);
            LOGD("时长：%d", pFormatCtx->streams[i]->duration);
        }
    }
    if (videoStream == -1) {
        LOGD("Didn't find a video stream.");
        return; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        LOGD("Codec not found.");
        return; // Codec not found
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGD("Could not open codec.");
        return; // Could not open codec
    }

    // 获取native window
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);

    // 获取视频宽高
    int videoWidth = pCodecCtx->width;
    int videoHeight = pCodecCtx->height;

    // 设置native window的buffer大小,可自动拉伸
    ANativeWindow_setBuffersGeometry(nativeWindow, videoWidth, videoHeight,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGD("Could not open codec.");
        return; // Could not open codec
    }

    // Allocate video frame
    AVFrame *pFrame = av_frame_alloc();

    // 用于渲染
    AVFrame *pFrameRGBA = av_frame_alloc();
    if (pFrameRGBA == NULL || pFrame == NULL) {
        LOGD("Could not allocate video frame.");
        return;
    }

    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height,
                                            1);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGBA->data, pFrameRGBA->linesize, buffer, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height, 1);

    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    struct SwsContext *sws_ctx = sws_getContext(pCodecCtx->width,
                                                pCodecCtx->height,
                                                pCodecCtx->pix_fmt,
                                                pCodecCtx->width,
                                                pCodecCtx->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_BILINEAR,
                                                NULL,
                                                NULL,
                                                NULL);

    int frameFinished;
    AVPacket packet;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream) {

            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // 并不是decode一次就可解码出一帧
            if (frameFinished) {

                // lock native window buffer
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                // 格式转换
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGBA->data, pFrameRGBA->linesize);

                // 获取stride
                uint8_t *dst = (uint8_t *) windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t *src = (pFrameRGBA->data[0]);
                int srcStride = pFrameRGBA->linesize[0];

                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < videoHeight; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }

                ANativeWindow_unlockAndPost(nativeWindow);
            }

        }
        av_packet_unref(&packet);
    }

    av_free(buffer);
    av_free(pFrameRGBA);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codecs
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    return;
}

/**
 * copy from stackoverflow
 * it show the flow of decode
 * @param env
 * @param type
 * @param surface
 */
JNIEXPORT void JNICALL
Java_com_example_ffmpeglib_FFmpegLib_decode(JNIEnv *env, jclass type, jobject surface) {
    AVFormatContext* ctx_format = nullptr;
    AVCodecContext* ctx_codec = nullptr;
    AVCodec* codec = nullptr;
    AVFrame* frame = av_frame_alloc();
    int stream_idx;
    SwsContext* ctx_sws = nullptr;
    const char* fin = "/sdcard/video.mkv";
    AVStream *vid_stream = nullptr;
    AVPacket* pkt = av_packet_alloc();

    av_register_all();

    if (int ret = avformat_open_input(&ctx_format, fin, nullptr, nullptr) != 0) {
        return;
    }
    if (avformat_find_stream_info(ctx_format, nullptr) < 0) {
        return; // Couldn't find stream information
    }
    av_dump_format(ctx_format, 0, fin, false);

    for (int i = 0; i < ctx_format->nb_streams; i++)
        if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream_idx = i;
            vid_stream = ctx_format->streams[i];
            break;
        }
    if (vid_stream == nullptr) {
        return;
    }

    codec = avcodec_find_decoder(vid_stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }
    ctx_codec = avcodec_alloc_context3(codec);

    if(avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar)<0)
        LOGD("");
    if (avcodec_open2(ctx_codec, codec, nullptr)<0) {
        return;
    }

    //av_new_packet(pkt, pic_size);

    while(av_read_frame(ctx_format, pkt) >= 0){
        if (pkt->stream_index == stream_idx) {
            int ret = avcodec_send_packet(ctx_codec, pkt);
            if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                LOGD("avcodec_send_packet: %d", ret);
                break;
            }
            while (ret  >= 0) {
                ret = avcodec_receive_frame(ctx_codec, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    LOGD("avcodec_receive_frame: %d", ret);
                    break;
                }
                LOGD("frame: %d", ctx_codec->frame_number);
            }
        }
        av_packet_unref(pkt);
    }

    avformat_close_input(&ctx_format);
    av_packet_unref(pkt);
    avcodec_free_context(&ctx_codec);
    avformat_free_context(ctx_format);
    return;
}

}