package net.theluckycoder.chess

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
