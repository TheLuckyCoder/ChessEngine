package net.theluckycoder.chess.model

data class Piece(
    @JvmField
    val pos: Int,
    @JvmField
    val type: Byte
) {

    fun getScore() = when (type) {
        PAWN -> 1 // Pawn
        KNIGHT -> 3 // Knight
        BISHOP -> 3 // Bishop
        ROOK -> 5 // Rook
        QUEEN -> 9 // Queen
        else -> 0
    }

    companion object {
        const val PAWN: Byte = 1
        const val KNIGHT: Byte = 2
        const val BISHOP: Byte = 3
        const val ROOK: Byte = 4
        const val QUEEN: Byte = 5
        const val KING: Byte = 6
    }
}
