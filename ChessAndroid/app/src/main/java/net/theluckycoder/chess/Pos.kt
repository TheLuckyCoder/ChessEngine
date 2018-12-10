package net.theluckycoder.chess


data class Pos(
    @JvmField
    val x: Short,
    @JvmField
    val y: Short
) {

    constructor() : this(-1, -1)
    constructor(x: Int, y: Int) : this(x.toShort(), y.toShort())

    val isValid
        get() = (x in 0..7 && y in 0..7)
}