package net.theluckycoder.chess

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

class GameManager(
    private val context: Context,
    private val listener: OnEventListener
) {

    interface OnEventListener {
        // Called before a move was made by the player or the engine
        fun onMove(gameState: State)

        // Called when a specific piece was moved
        fun onPieceMoved(startPos: Pos, destPos: Pos, isPlayerWhite: Boolean)

        fun redrawBoard(isPlayerWhite: Boolean)

        fun redrawPieces(newPieces: List<Piece>, isPlayerWhite: Boolean)
    }

    private var initialized = false
    private var isPlayerWhite = true

    var basicStatsEnabled = false
    var advancedStatsEnabled = false
    val isWorking
        get() = Native.isWorking()

    fun selectPiece(pos: Pos) = Native.getPossibleMoves(pos) ?: emptyArray()

    fun initBoard(restartGame: Boolean, playerWhite: Boolean = true) {
        initBoardNative(restartGame, playerWhite)

        if (initialized) {
            if (isPlayerWhite != playerWhite) {
                isPlayerWhite = playerWhite
                listener.redrawBoard(playerWhite)
            }

            listener.redrawPieces(getPiecesList(), playerWhite)
        } else {
            initialized = true

            listener.redrawBoard(isPlayerWhite)

            isPlayerWhite = if (SaveManager.loadFromFile(context)) {
                Native.isPlayerWhite()
            } else {
                initBoardNative(true, playerWhite)
                listener.redrawPieces(getPiecesList(), playerWhite)
                playerWhite
            }
        }
    }

    fun updateSettings(settings: Settings) {
        Native.setSettings(
            settings.baseSearchDepth,
            settings.threadCount,
            settings.cacheSize,
            settings.performQuiescenceSearch
        )
    }

    fun makeMove(startPos: Pos, destPos: Pos) {
        Native.enableStats(advancedStatsEnabled)
        Native.movePiece(
            startPos.x.toByte(),
            startPos.y.toByte(),
            destPos.x.toByte(),
            destPos.y.toByte()
        )
    }

    private fun getPiecesList() = Native.getPieces().filter { it.type.toInt() != 0 }

    @Suppress("unused") // Called by native code
    private fun callback(gameState: Int, shouldRedrawPieces: Boolean, moves: Array<PosPair>) {
        val state = when (gameState) {
            1 -> State.WINNER_WHITE
            2 -> State.WINNER_BLACK
            3 -> State.DRAW
            4 -> State.WHITE_IN_CHESS
            5 -> State.BLACK_IN_CHESS
            else -> State.NONE
        }

        GlobalScope.launch(Dispatchers.Default) { SaveManager.saveToFile(context) }

        GlobalScope.launch(Dispatchers.Main.immediate) {
            listener.onMove(state)

            moves.forEach {
                listener.onPieceMoved(
                    Pos(it.startX, it.startY),
                    Pos(it.destX, it.destY),
                    isPlayerWhite
                )
            }

            if (shouldRedrawPieces) {
                delay(260L)
                listener.redrawPieces(getPiecesList(), isPlayerWhite)
            }
        }
    }

    private external fun initBoardNative(restartGame: Boolean, isPlayerWhite: Boolean)
}
