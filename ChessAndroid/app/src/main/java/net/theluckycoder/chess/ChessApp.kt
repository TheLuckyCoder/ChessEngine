package net.theluckycoder.chess

import android.app.Application
import android.content.Context
import android.util.Log
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import net.theluckycoder.chess.utils.AppPreferences
import java.io.File
import kotlin.concurrent.thread

@Suppress("unused")
class ChessApp : Application() {

    override fun onCreate() {
        super.onCreate()
        AppPreferences.init(applicationContext)

        copyBook()
    }

    private fun copyBook() {
        thread {
            val dest = File(filesDir, "Book.bin")

            assets.open("Book.bin").use { input ->
                dest.outputStream().use { output ->
                    input.copyTo(output)
                }
            }

            Native.initBook(dest.absolutePath)
        }
    }

    companion object {
        init {
            System.loadLibrary("chess")
        }
    }
}
