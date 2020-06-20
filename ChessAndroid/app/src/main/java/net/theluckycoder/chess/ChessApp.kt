package net.theluckycoder.chess

import android.app.Application
import net.theluckycoder.chess.utils.AppPreferences

@Suppress("unused")
class ChessApp : Application() {

    override fun onCreate() {
        super.onCreate()
        AppPreferences.init(applicationContext)
    }

    companion object {
        init {
            System.loadLibrary("chess")
        }
    }
}
