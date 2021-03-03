package net.theluckycoder.chess.utils

import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.graphics.Point
import android.net.Uri
import android.os.Build
import net.theluckycoder.chess.R

@Suppress("DEPRECATION")
fun Activity.getWindowSize(): Point {
    val point = Point()
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R)
        windowManager.defaultDisplay.getSize(point)
    else
        display?.getRealSize(point)
    return point
}

@Suppress("DEPRECATION")
fun Context.getColorCompat(color: Int): Int {
    return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
        getColor(color)
    } else resources.getColor(color)
}

fun Byte.toBoolean() = this != 0.toByte()

fun Context.browseUrl(url: String): Boolean {
    return try {
        val intent = Intent(Intent.ACTION_VIEW).apply {
            data = Uri.parse(url)
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        }

        startActivity(intent)
        true
    } catch (e: ActivityNotFoundException) {
        e.printStackTrace()
        false
    }
}
