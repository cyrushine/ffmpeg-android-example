package com.example.ndkffmpeg

import android.content.Context
import android.content.pm.PackageManager
import android.support.v4.app.ActivityCompat

object Util {

    fun permissionGranted(ctx: Context, permissions: Array<String>): Boolean {
        for (permission in permissions) {
            if (ActivityCompat.checkSelfPermission(ctx, permission) == PackageManager.PERMISSION_DENIED)
                return false
        }
        return true
    }
}