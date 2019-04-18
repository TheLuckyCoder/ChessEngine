package net.theluckycoder.chess

import android.content.Context
import java.io.FileNotFoundException

object SaveManager {

    private const val SAVE_FILE_NAME = "save.txt"

    fun saveToFile(context: Context) {
        val moves = Native.saveMoves()

        context.openFileOutput(SAVE_FILE_NAME, Context.MODE_PRIVATE).writer().use {
            it.write(moves)
        }
    }

    fun loadFromFile(context: Context) {
        var moves = ""

        try {
            context.openFileInput(SAVE_FILE_NAME).reader().use {
                moves = it.readText()
            }
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
        }

        Native.loadMoves(moves)
    }
}
