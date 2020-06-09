package net.theluckycoder.chess

import android.app.Activity
import net.theluckycoder.chess.model.*

class GameManager(
    private val activity: Activity,
    private val listener: OnEventListener
) {

    interface OnEventListener {
        // Called before a move was made by the player or the engine
        fun onMove(gameState: GameState)

        // Called when a specific piece was moved
        fun onPieceMoved(startPos: Pos, destPos: Pos, isPlayerWhite: Boolean)

        fun redrawBoard(isPlayerWhite: Boolean)

        fun redrawPieces(newPieces: List<Piece>, isPlayerWhite: Boolean)
    }

    private var initialized = false
    private var isPlayerWhite = true

    var basicStatsEnabled = false
    var advancedStatsEnabled = false
        set(value) {
            if (field != value) {
                field = value
                Native.enableStats(value)
            }
        }
    val isWorking
        get() = Native.isWorking()

    fun getPossibleMoves(pos: Pos) = Native.getPossibleMoves(pos.toByte()).map { Move(it) }

    fun initBoard(playerWhite: Boolean = true) {
        if (initialized) {
            isPlayerWhite = playerWhite
            initBoardNative(isPlayerWhite)
        } else {
            // First time it is called, load the last game
            initialized = true

            initBoardNative(isPlayerWhite)

            isPlayerWhite =
                if (SaveManager.loadFromFile(activity)) Native.isPlayerWhite() else playerWhite
        }

        listener.redrawBoard(isPlayerWhite)
        listener.redrawPieces(getPiecesList(), isPlayerWhite)
    }

    fun updateSettings(engineSettings: EngineSettings) {
        Native.setSettings(
            engineSettings.searchDepth,
            engineSettings.threadCount,
            engineSettings.cacheSize,
            engineSettings.doQuietSearch
        )
    }

    fun makeMove(move: Move) {
        Native.makeMove(move.content)
    }

    private fun getPiecesList() = Native.getPieces().filter { it.type.toInt() != 0 }

    @Suppress("unused") // Called by native code
    private fun callback(gameState: Int, shouldRedrawPieces: Boolean, moves: Array<PosPair>) {
        val state = when (gameState) {
            1 -> GameState.WINNER_WHITE
            2 -> GameState.WINNER_BLACK
            3 -> GameState.DRAW
            4 -> GameState.WHITE_IN_CHESS
            5 -> GameState.BLACK_IN_CHESS
            else -> GameState.NONE
        }

        SaveManager.saveToFileAsync(activity)

        activity.runOnUiThread {
            listener.onMove(state)

            moves.forEach {
                listener.onPieceMoved(
                    Pos(it.startX, it.startY),
                    Pos(it.destX, it.destY),
                    isPlayerWhite
                )
            }
        }

        if (shouldRedrawPieces) {
            // Wait for any piece animations to occur to make the redraw smother, there probably is a better way of doing this
            Thread.sleep(260L)

            activity.runOnUiThread {
                listener.redrawPieces(getPiecesList(), isPlayerWhite)
            }
        }
    }

    private external fun initBoardNative(isPlayerWhite: Boolean)
}
