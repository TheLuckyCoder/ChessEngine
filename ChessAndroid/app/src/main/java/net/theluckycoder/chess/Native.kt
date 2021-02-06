package net.theluckycoder.chess

import net.theluckycoder.chess.model.Move
import net.theluckycoder.chess.model.Piece

object Native {

    external fun isWorking(): Boolean

    external fun isPlayerWhite(): Boolean

    // region Stats

    external fun getSearchTime(): Double

    external fun getCurrentBoardValue(): Int

    external fun getAdvancedStats(): String

    // endregion Stats

    external fun getPieces(): Array<Piece>

    external fun getPossibleMoves(square: Byte): Array<Move>

    external fun makeMove(move: Int)

    external fun forceMove()

    external fun stopSearch()

    external fun enableStats(enabled: Boolean)

    external fun setSettings(
        baseSearchDepth: Int,
        threadCount: Int,
        cacheSizeInMb: Int,
        performQuiescenceSearch: Boolean
    )

    external fun undoMoves(): Boolean

    external fun loadFen(playerWhite: Boolean, position: String): Boolean
    external fun loadMoves(moves: String)
    external fun getFen(): String

    external fun saveMoves(): String?

    external fun perftTest()
    external fun evaluationTest(): String?
}
