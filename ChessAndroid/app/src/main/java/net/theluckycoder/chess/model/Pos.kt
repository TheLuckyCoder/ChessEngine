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
