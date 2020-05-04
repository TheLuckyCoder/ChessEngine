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

data class Settings(
    val searchDepth: Int,
    val threadCount: Int,
    val cacheSize: Int,
    val doQuietSearch: Boolean
) {

    companion object {
        private val MAX_THREAD_COUNT = Runtime.getRuntime().availableProcessors()

        fun create(
            searchDepth: Int,
            threadCount: Int,
            cacheSize: Int,
            doQuietSearch: Boolean
        ): Settings {
            val threads = when {
                threadCount > MAX_THREAD_COUNT -> MAX_THREAD_COUNT
                threadCount < 1 -> MAX_THREAD_COUNT - 1
                else -> threadCount
            }
            return Settings(
                if (searchDepth < 0) 1 else searchDepth,
                threads,
                cacheSize,
                doQuietSearch
            )
        }
    }
}
