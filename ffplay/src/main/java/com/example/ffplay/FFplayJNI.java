package com.example.ffplay;

import android.view.Surface;

public class FFplayJNI {

    static {
        System.loadLibrary("ffplay");
    }

    private String filePath;
    private Surface surface;

    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    public Surface getSurface() {
        return surface;
    }

    public void setSurface(Surface surface) {
        this.surface = surface;
    }

    public native void testLog(String msg);

    public native void start();
}
