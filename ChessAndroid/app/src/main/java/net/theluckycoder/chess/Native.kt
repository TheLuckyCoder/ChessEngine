package net.theluckycoder.chess

import net.theluckycoder.chess.model.EngineSettings
import net.theluckycoder.chess.model.IndexedPiece
import net.theluckycoder.chess.model.Move

object Native {

    external fun isWorking(): Boolean

    external fun isPlayerWhite(): Boolean

    external fun getPieces(): Array<IndexedPiece>

    external fun getPossibleMoves(square: Byte): Array<Move>

    external fun makeMove(move: Int)

    external fun forceMove()

    external fun stopSearch()

    external fun enableStats(enabled: Boolean)

    external fun getSearchOptions(): EngineSettings

    external fun setSearchOptions(
        searchDepth: Int,
        quietSearch: Boolean,
        threadCount: Int,
        hashSizeMb: Int,
        searchTime: Long,
    )

    external fun undoMoves(): Boolean
    external fun redoMoves(): Boolean

    external fun loadFen(playerWhite: Boolean, position: String): Boolean
    external fun loadMoves(moves: String)
    external fun getCurrentFen(): String

    external fun saveMoves(): String?

    external fun initBook(bookPath: String)

    // region Tests

    external fun perftTest()
    external fun evaluationTest(): String?

    // endregion Tests
}
