package net.theluckycoder.chess

import android.content.Context
import java.io.File
import java.io.FileNotFoundException
import java.util.concurrent.Executors

object SaveManager {

    private const val SAVE_FILE_NAME = "moves.txt"

    private val executor = Executors.newSingleThreadExecutor()

    fun saveToFileAsync(context: Context) {
        val content = Native.saveMoves() ?: return
        val appContext = context.applicationContext
        executor.execute { saveToFile(content, appContext) }
    }

    private fun saveToFile(content: String, context: Context) {
        val file = File(context.filesDir, SAVE_FILE_NAME)

        if (!file.exists())
            file.createNewFile()

        context.openFileOutput(SAVE_FILE_NAME, Context.MODE_PRIVATE).writer().use {
            it.write(content)
        }
    }

    fun loadFromFile(context: Context): Boolean {
        try {
            context.openFileInput(SAVE_FILE_NAME).reader().use {
                val content = it.readText()

                return if (content.isNotBlank()) {
                    Native.loadMoves(content)
                    true
                } else false
            }
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
            return false
        }
    }
}
