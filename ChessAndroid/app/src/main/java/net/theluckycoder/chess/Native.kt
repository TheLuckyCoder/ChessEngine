package net.theluckycoder.chess

object Native {
    @JvmStatic
    external fun isWorking(): Boolean

    @JvmStatic
    external fun isPlayerWhite(): Boolean

    // region Stats

    @JvmStatic
    external fun getSearchTime(): Double

    @JvmStatic
    external fun getCurrentBoardValue(): Int

    @JvmStatic
    external fun getBestMoveFound(): Int

    @JvmStatic
    external fun getAdvancedStats(): String

    // endregion Stats

    @JvmStatic
    external fun getPieces(): Array<Piece>

    @JvmStatic
    external fun getPossibleMoves(selected: Pos): Array<Pos>?

    @JvmStatic
    external fun movePiece(selectedX: Byte, selectedY: Byte, destX: Byte, destY: Byte): Boolean

    @JvmStatic
    external fun enableStats(enabled: Boolean)

    @JvmStatic
    external fun setSettings(
        baseSearchDepth: Int,
        threadCount: Int,
        cacheSizeInMb: Int,
        performQuiescenceSearch: Boolean
    )

    @JvmStatic
    external fun undoMoves()

    @JvmStatic
    external fun loadMoves(moves: String)

    @JvmStatic
    external fun saveMoves(): String

    @JvmStatic
    external fun perft(depth: Int)
}
