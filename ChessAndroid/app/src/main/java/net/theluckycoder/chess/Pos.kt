package net.theluckycoder.chess

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