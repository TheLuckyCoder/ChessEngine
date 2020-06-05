package net.theluckycoder.chess.model

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

    fun toInt() = y * 8 + x

    fun toByte() = toInt().toByte()
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
