package net.theluckycoder.chess.utils

import android.content.Context
import android.os.Build

@Suppress("DEPRECATION")
fun Context.getColorCompat(color: Int): Int {
    return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
        getColor(color)
    } else resources.getColor(color)
}

fun Byte.toBoolean() = this != 0.toByte()

