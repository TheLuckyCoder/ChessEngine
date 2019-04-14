package net.theluckycoder.chess

object Native {
    @JvmStatic
    external fun isPlayerWhite(): Boolean

    @JvmStatic
    external fun isWorking(): Boolean

    @JvmStatic
    external fun getStats(): String

    @JvmStatic
    external fun getBoardValue(): Int

    @JvmStatic
    external fun getPieces(): Array<Piece>

    @JvmStatic
    external fun getPossibleMoves(selected: Pos): Array<Pos>?

    @JvmStatic
    fun movePiece(selected: Pos, dest: Pos): Boolean = movePiece(selected.x, selected.y, dest.x, dest.y)

    @JvmStatic
    external fun movePiece(selectedX: Byte, selectedY: Byte, destX: Byte, destY: Byte): Boolean

    @JvmStatic
    external fun enableStats(enabled: Boolean)

    @JvmStatic
    fun setSettings(settings: Settings) =
        setSettings(settings.baseSearchDepth, settings.threadCount, settings.cacheSize)

    @JvmStatic
    external fun setSettings(baseSearchDepth: Int, threadCount: Int, cacheSizeInMb: Int)

    @JvmStatic
    external fun loadMoves(moves: String)

    @JvmStatic
    external fun saveMoves(): String
}
