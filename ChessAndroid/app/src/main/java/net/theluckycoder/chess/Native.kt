package net.theluckycoder.chess

object Native {
    @JvmStatic
    external fun isPlayerWhite(): Boolean

    @JvmStatic
    external fun getNumberOfEvaluatedBoards(): Int

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

    @JvmStatic
    external fun loadFromJson(json: String)

    @JvmStatic
    external fun saveToJson(): String

    @JvmStatic
    external fun loadMoves(moves: String)

    @JvmStatic
    external fun saveMoves(): String
}
