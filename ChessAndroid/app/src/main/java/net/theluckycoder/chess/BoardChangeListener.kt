package net.theluckycoder.chess

import androidx.annotation.Keep

interface BoardChangeListener {

    @Suppress("unused")
    @Keep // Called by native code
    fun boardChanged(gameStateInt: Int)
}

