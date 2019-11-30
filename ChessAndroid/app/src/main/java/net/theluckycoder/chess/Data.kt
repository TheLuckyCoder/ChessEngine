package net.theluckycoder.chess

enum class State {
    NONE,
    WINNER_WHITE,
    WINNER_BLACK,
    DRAW,
    WHITE_IN_CHESS,
    BLACK_IN_CHESS,
}

data class Piece(
    @JvmField
    val x: Int,
    @JvmField
    val y: Int,
    @JvmField
    val type: Byte
) {

    fun getScore() = when (type.toInt()) {
        1 -> 1 // Pawn
        2 -> 3 // Knight
        3 -> 3 // Bishop
        4 -> 5 // Rook
        5 -> 9 // Queen
        else -> 0
    }
}

data class Pos(
    @JvmField
    val x: Int,
    @JvmField
    val y: Int
) {

    constructor() : this(8, 8)
    constructor(square: Byte) : this(square % 8, square / 8)

    fun invertIf(invert: Boolean) = Pos(x, if (invert) 7 - y else y)

    val isValid
        get() = (x in 0..7 && y in 0..7)
}

data class PosPair(
    @JvmField
    val startX: Int,
    @JvmField
    val startY: Int,
    @JvmField
    val destX: Int,
    @JvmField
    val destY: Int
)

class Settings(
    baseSearchDepth: Int,
    threadCount: Int,
    val cacheSize: Int,
    val performQuiescenceSearch: Boolean
) {

    val baseSearchDepth = if (baseSearchDepth < 0) 1 else baseSearchDepth
    val threadCount: Int

    init {
        val maxThreadCount = Runtime.getRuntime().availableProcessors()
        this.threadCount = when {
            threadCount > maxThreadCount -> maxThreadCount
            threadCount < 1 -> maxThreadCount - 1
            else -> threadCount
        }
    }
}
