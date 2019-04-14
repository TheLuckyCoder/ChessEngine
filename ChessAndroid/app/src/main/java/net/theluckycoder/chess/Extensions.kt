package net.theluckycoder.chess

import android.content.Context
import android.os.Build

@Suppress("DEPRECATION")
fun getColor(context: Context, color: Int): Int = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
    context.getColor(color)
} else context.resources.getColor(color)

fun Boolean.toInt() = if (this) 1 else 0
