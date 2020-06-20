package net.theluckycoder.chess.model

data class BoardAppearance(
    val whiteTile: Int,
    val blackTile: Int,
    val possible: Int,
    val lastMoved: Int,
    val kingInCheck: Int,
    val showCoordinates: Boolean
)
