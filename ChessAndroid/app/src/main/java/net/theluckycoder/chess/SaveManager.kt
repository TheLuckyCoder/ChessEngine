package net.theluckycoder.chess

import android.content.Context
import java.io.FileNotFoundException

object SaveManager {

    private const val SAVE_FILE_NAME = "moves.txt"

    fun saveToFile(context: Context) {
        Native.saveMoves()?.let { moves ->
            context.openFileOutput(SAVE_FILE_NAME, Context.MODE_PRIVATE).writer().use {
                it.write(moves)
            }
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
