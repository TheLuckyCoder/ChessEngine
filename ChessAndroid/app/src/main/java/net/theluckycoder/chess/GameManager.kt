package net.theluckycoder.chess

import android.content.Context
import kotlinx.coroutines.*

class GameManager(
    private val appContext: Context,
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

    var statsEnabled = false
    var isWorking = false
        private set

    fun selectPiece(pos: Pos) = Native.getPossibleMoves(pos) ?: emptyArray()

    fun initBoard(restartGame: Boolean, isPlayerWhite: Boolean = true) {
        val isOldPlayerWhite = this.isPlayerWhite
        this.isPlayerWhite = isPlayerWhite
        isWorking = !isPlayerWhite

        initBoardNative(restartGame, isPlayerWhite)

        if (initialized) {
            if (isOldPlayerWhite != isPlayerWhite)
                listener.redrawBoard(isPlayerWhite)
        } else {
            initialized = true
            listener.redrawBoard(isPlayerWhite)
            //SaveManager.loadFromFile(appContext)
        }

        val pieces = Native.getPieces().filter { it.type.toInt() != 0 }
        listener.redrawPieces(pieces, isPlayerWhite)
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
        Native.enableStats(statsEnabled)
        Native.movePiece(startPos.x, startPos.y, destPos.x, destPos.y)
    }

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

        isWorking = !isWorking

        GlobalScope.launch(Dispatchers.Default) { SaveManager.saveToFile(appContext) }

        GlobalScope.launch(Dispatchers.Main) {
            if (shouldRedrawPieces)
                listener.redrawBoard(isPlayerWhite)

            listener.onMove(state)

            moves.forEach {
                listener.onPieceMoved(
                    Pos(it.startX, it.startY),
                    Pos(it.destX, it.destY),
                    isPlayerWhite
                )
            }
        }
    }

    private external fun initBoardNative(restartGame: Boolean, isPlayerWhite: Boolean)
}