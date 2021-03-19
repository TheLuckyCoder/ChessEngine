package net.theluckycoder.chess

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import net.theluckycoder.chess.model.Move
import java.io.File
import java.io.FileNotFoundException

object SaveManager {

    private const val SAVE_FILE_NAME = "moves.sav"

    suspend fun saveToFileAsync(context: Context, playerWhite: Boolean, moves: List<Move>) {
        if (moves.isEmpty()) return

        val content = buildString {
            append(if (playerWhite) 1 else 0)
            moves.forEach {
                append('\n').append(it.content.toString())
            }
        }

        val appContext = context.applicationContext
        withContext(Dispatchers.IO) { saveToFile(appContext, content) }
    }

    private fun saveToFile(context: Context, content: String) {
        val file = File(context.filesDir, SAVE_FILE_NAME)

        if (!file.exists())
            file.createNewFile()

        context.openFileOutput(SAVE_FILE_NAME, Context.MODE_PRIVATE).bufferedWriter().use {
            it.write(content)
        }
    }

    fun loadFromFile(context: Context): Boolean {
        try {
            context.openFileInput(SAVE_FILE_NAME).bufferedReader().use { reader ->
                val moves = reader.readLines().map { it.toInt() }.toMutableList()
                val playerWhite = moves.removeFirst() == 1

                return if (moves.isNotEmpty()) {
                    Native.loadMoves(playerWhite, moves.toIntArray())
                    true
                } else false
            }
        } catch (e: FileNotFoundException) {
            e.printStackTrace()
            return false
        }
    }
}
