package net.theluckycoder.chess.utils

import android.app.Activity
import android.content.Context
import android.graphics.Point
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

fun Context.getActionAndStatusBarHeight(): Int {
    val styledAttributes = theme.obtainStyledAttributes(intArrayOf(R.attr.actionBarSize))

    val actionBarHeight = styledAttributes.getDimension(0, 0f).toInt()
    styledAttributes.recycle()

    val resourceId = resources.getIdentifier("status_bar_height", "dimen", "android")
    val statusBarHeight = if (resourceId > 0)
        resources.getDimensionPixelSize(resourceId) else 0
    return actionBarHeight + statusBarHeight
}

fun Byte.toBoolean() = this != 0.toByte()

