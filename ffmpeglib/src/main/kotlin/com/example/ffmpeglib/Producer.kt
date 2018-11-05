package com.example.ffmpeglib

import android.graphics.Bitmap
import android.view.Surface
import java.util.concurrent.ArrayBlockingQueue

/**
 * 使用生产者消费者模式来渲染视频
 */
class Producer(private val videoPath: String, private val surface: Surface) {

    private val mBlockingQueue = ArrayBlockingQueue<Bitmap>(10)

    init {}

}