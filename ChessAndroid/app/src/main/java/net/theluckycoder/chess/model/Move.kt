package net.theluckycoder.chess.model

import net.theluckycoder.chess.utils.toBoolean
import kotlin.experimental.and

@Suppress("unused", "MemberVisibilityCanBePrivate")
data class Move(
    val content: Int,
    val from: Byte,
    val to: Byte,
    val pieceType: Byte,
    val capturedPieceType: Byte,
    val promotedPieceType: Byte,
    private val internalFlags: Byte
) {
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

    val piece: Piece
        get() = Piece(from.toInt(), pieceType)

    val flags = Flags(internalFlags.toInt())
}
