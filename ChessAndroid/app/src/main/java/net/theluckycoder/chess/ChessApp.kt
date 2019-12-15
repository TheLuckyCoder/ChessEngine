package net.theluckycoder.chess

import android.app.Application

@Suppress("unused")
class ChessApp : Application() {

    companion object {
        init {
            System.loadLibrary("chess")
        }
    }
}
