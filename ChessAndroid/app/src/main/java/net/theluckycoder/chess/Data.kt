package net.theluckycoder.chess

data class Piece(
    @JvmField
    val x: Byte,
    @JvmField
    val y: Byte,
    @JvmField
    val type: Byte
)

data class Pos(
    @JvmField
    val x: Byte,
    @JvmField
    val y: Byte
) {

    constructor() : this(8, 8)
    constructor(x: Int, y: Int) : this(x.toByte(), y.toByte())

    val isValid
        get() = (x in 0..7 && y in 0..7)
}

data class PosPair(
    @JvmField
    val startX: Byte,
    @JvmField
    val startY: Byte,
    @JvmField
    val destX: Byte,
    @JvmField
    val destY: Byte
)

class Settings(
    val baseSearchDepth: Int = 4,
    threadCount: Int = Runtime.getRuntime().availableProcessors() - 1
) {

    val threadCount: Int

    init {
        val maxThreadCount = Runtime.getRuntime().availableProcessors()
        this.threadCount = when {
            threadCount > maxThreadCount -> maxThreadCount
            threadCount < 1 -> 1
            else -> threadCount
        }
    }
}
