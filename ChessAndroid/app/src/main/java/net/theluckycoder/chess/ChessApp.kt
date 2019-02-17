package net.theluckycoder.chess

import android.app.Application

class ChessApp : Application() {
    companion object {
        init {
            System.loadLibrary("chess")
        }
    }
}
