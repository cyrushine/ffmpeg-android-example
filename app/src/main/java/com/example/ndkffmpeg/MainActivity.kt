package com.example.ndkffmpeg

import android.Manifest
import android.content.Intent
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.support.v4.app.ActivityCompat
import android.util.Log
import android.view.*
import android.widget.TextView

import com.example.ffmpeglib.FFmpegLib
import com.example.ffmpeglib.Producer
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    private var mSurface: Surface? = null
    private val mPermissions = arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        surfaceView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceChanged(holder: SurfaceHolder?, format: Int, width: Int, height: Int) {

            }

            override fun surfaceDestroyed(holder: SurfaceHolder?) {
            }

            override fun surfaceCreated(holder: SurfaceHolder?) {
                mSurface = holder?.surface
                playVideo()
            }
        })

        if (!Util.permissionGranted(this, mPermissions)) {
            ActivityCompat.requestPermissions(this, mPermissions, PLAY_VIDEO)
        }
    }

    fun playVideo() {
        if (mSurface != null && Util.permissionGranted(this, mPermissions)) {
            Producer(VIDEO_PATH, mSurface!!)
            val play = {FFmpegLib.renderToWindowByMe(mSurface)}
            Thread(play).start()
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        when (requestCode) {
            PLAY_VIDEO -> playVideo()
            else -> {
                super.onRequestPermissionsResult(requestCode, permissions, grantResults)
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        MenuInflater(this).inflate(R.menu.main_menu, menu);
        return true;
    }

    override fun onOptionsItemSelected(item: MenuItem?): Boolean {
        when (item?.itemId) {
            R.id.select_file -> {
                val intent = Intent(Intent.ACTION_PICK);
                intent.type = "video/x-msvideo"
                startActivityForResult(intent, SELECT_FILE)
                return true
            }

            else -> return super.onOptionsItemSelected(item)
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        when (requestCode) {
            SELECT_FILE -> {
                Log.d(TAG, data?.data?.toString() ?: "null")
            }

            else -> super.onActivityResult(requestCode, resultCode, data)
        }
    }

    companion object {
        val SELECT_FILE = 1
        val TAG = "MainActivity"
        val PLAY_VIDEO = 1

        val VIDEO_PATH = "/sdcard/video.mkv"
    }
}
