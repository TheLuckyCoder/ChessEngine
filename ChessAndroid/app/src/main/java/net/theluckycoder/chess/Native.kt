package net.theluckycoder.chess

import net.theluckycoder.chess.model.IndexedPiece
import net.theluckycoder.chess.model.Move
import net.theluckycoder.chess.model.SearchOptions
import kotlin.time.ExperimentalTime

object Native {

    external fun initBoard(boardChangeListener: BoardChangeListener, isPlayerWhite: Boolean)
    external fun initBook(bookPath: String)

    external fun loadFen(playerWhite: Boolean, fen: String): Boolean
    external fun loadFenMoves(playerWhite: Boolean, fen: String, moves: IntArray)

    external fun isEngineWorking(): Boolean
    external fun isPlayerWhite(): Boolean
    external fun isPlayersTurn(): Boolean

    external fun getPieces(): Array<IndexedPiece>
    external fun getPossibleMoves(square: Byte): Array<Move>

    fun makeMove(move: Move) = makeMove(move.content)
    external fun makeMove(move: Int)
    external fun makeEngineMove()

    external fun stopSearch()

    external fun getSearchOptions(): SearchOptions

    @OptIn(ExperimentalTime::class)
    fun setSearchOptions(options: SearchOptions) = setSearchOptions(
        options.searchDepth,
        options.quietSearch,
        options.threadCount,
        options.hashSize,
        options.searchTime.toLongMilliseconds(),
    )

    private external fun setSearchOptions(
        searchDepth: Int, quietSearch: Boolean,
        threadCount: Int, hashSizeMb: Int,
        searchTime: Long,
    )

    external fun undoMoves()
    external fun redoMoves()

    external fun getCurrentFen(): String
    external fun getStartFen(): String
    external fun getMovesHistory(): Array<Move>
    external fun getCurrentMoveIndex(): Int

    // region Tests

    external fun perftTests()
    external fun evaluationTests(): String?

    // endregion Tests
}
