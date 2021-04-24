package net.theluckycoder.chess

import android.app.Application
import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import net.theluckycoder.chess.utils.SettingsDataStore
import java.io.File

@Suppress("unused")
class ChessApp : Application() {

    override fun onCreate() {
        super.onCreate()

        GlobalScope.launch(Dispatchers.IO) {
            val dataStore = SettingsDataStore.get(this@ChessApp)

            try {
                copyBook()
            } catch (e: Exception) {
                e.printStackTrace()
            }

            launch {
                if (dataStore.firstStart().first()) {
                    // Set the default Engine Settings from native code
                    val engineSettings = Native.getSearchOptions()
                    dataStore.setEngineSettings(engineSettings)
                    dataStore.setFirstStart(false)
                }
            }
        }
    }

    private fun copyBook() {
        val dest = getBookPath(this)

        assets.open(BOOK_NAME).use { input ->
            dest.outputStream().use { output ->
                input.copyTo(output)
            }
        }

        Native.initBook(dest.absolutePath)
    }

    companion object {
        init {
            System.loadLibrary("chess")
        }

        const val BOOK_NAME = "OpeningBook.bin"

        fun getBookPath(context: Context): File = File(context.filesDir, BOOK_NAME)
    }
}
