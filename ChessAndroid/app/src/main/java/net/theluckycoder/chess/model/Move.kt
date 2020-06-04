package net.theluckycoder.chess.model

import kotlin.experimental.and

fun Byte.toBoolean() = this != 0.toByte()

@Suppress("unused", "MemberVisibilityCanBePrivate")
data class Move(val content: Long) {
    companion object {
        const val CAPTURE: Byte = 1
        const val PROMOTION: Byte = 1 shl 2
        const val KSIDE_CASTLE: Byte = 1 shl 3
        const val QSIDE_CASTLE: Byte = 1 shl 4
        const val DOUBLE_PAWN_PUSH: Byte = 1 shl 5
        const val EN_PASSANT: Byte = 1 shl 6
    }

    class Flags(flags: Int) {
        private val flags = (flags and 0x7F).toByte()

        val capture: Boolean
            get() = (flags and CAPTURE).toBoolean()

        val promotion: Boolean
            get() = (flags and PROMOTION).toBoolean()

        val kSideCastle: Boolean
            get() = (flags and KSIDE_CASTLE).toBoolean()

        val qSideCastle: Boolean
            get() = (flags and QSIDE_CASTLE).toBoolean()

        val castle: Boolean
            get() = kSideCastle || qSideCastle

        val doublePawnPush: Boolean
            get() = (flags and DOUBLE_PAWN_PUSH).toBoolean()

        val enPassant: Boolean
            get() = (flags and EN_PASSANT).toBoolean()
    }

    val from: Byte
        get() = ((content ushr 9) and 0x3F).toByte()

    val to: Byte
        get() = ((content ushr 15) and 0x3F).toByte()

    val pieceType = (content and 7).toByte()

    val piece: Piece
        get() = Piece(from.toInt(), pieceType)

    val capturedPieceType: Byte
        get() = ((content ushr 3) and 7).toByte()

    val capturedPiece: Piece
        get() = Piece(to.toInt(), capturedPieceType)

    val promotedPieceType: Byte
        get() = ((content ushr 6) and 7).toByte()

    val flags: Flags
        get() = Flags((content ushr 22).toInt())
}
