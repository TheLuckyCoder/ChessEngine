package net.theluckycoder.chess

import android.app.Application
import java.io.File
import kotlin.concurrent.thread

@Suppress("unused")
class ChessApp : Application() {

    override fun onCreate() {
        super.onCreate()

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
