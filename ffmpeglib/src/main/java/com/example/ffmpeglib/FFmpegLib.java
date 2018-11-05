package com.example.ffmpeglib;

import android.graphics.Bitmap;
import android.view.Surface;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class FFmpegLib {

    static {
        System.loadLibrary("ffmpeg-lib");
    }

    public native static void renderToWindowByMe(Surface surface);
}
