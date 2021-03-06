package net.theluckycoder.chess

import android.app.Application
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import net.theluckycoder.chess.utils.SettingsDataStore
import java.io.File
import kotlin.concurrent.thread

@Suppress("unused")
class ChessApp : Application() {

    override fun onCreate() {
        super.onCreate()

        GlobalScope.launch(Dispatchers.IO) {
            val dataStore = SettingsDataStore(this@ChessApp)
            if (dataStore.firstStart().first()) {
                // Set the default Engine Settings from native code
                val engineSettings = Native.getSearchOptions()
                dataStore.setEngineSettings(engineSettings)
                dataStore.setFirstStart(false)
            }
        }
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
