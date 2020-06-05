package net.theluckycoder.chess.model

data class ChessColors(
    val whiteTile: Int,
    val blackTile: Int,
    val possible: Int,
    val lastMoved: Int,
    val kingInCheck: Int
)
