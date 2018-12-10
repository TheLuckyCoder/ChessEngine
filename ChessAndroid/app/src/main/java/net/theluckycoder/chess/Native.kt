package net.theluckycoder.chess

object Native {
    @JvmStatic
    external fun isWhiteAtBottom(): Boolean

    @JvmStatic
    external fun isWorking(): Boolean

    @JvmStatic
    external fun getPieces(): Array<Piece>

    @JvmStatic
    external fun getPossibleMoves(selected: Pos): Array<Pos>?

    @JvmStatic
    fun movePiece(selected: Pos, dest: Pos): Boolean = movePiece(selected.x, selected.y, dest.x, dest.y)

    @JvmStatic
    external fun movePiece(selectedX: Short, selectedY: Short, destX: Short, destY: Short): Boolean
}
