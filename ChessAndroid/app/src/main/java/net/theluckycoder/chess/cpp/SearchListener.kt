package net.theluckycoder.chess.cpp

import androidx.annotation.Keep

interface SearchListener {

    @Keep // Called by native code
    fun onFinish(success: Boolean)
}
