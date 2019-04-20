package net.theluckycoder.chess.utils

import net.theluckycoder.chess.Piece

class CapturedPieces {

    private val whitePieces = ArrayList<Int>()
    private val blackPieces = ArrayList<Int>()
    var whiteScore = 0
        private set
    var blackScore = 0
        private set

    fun addWhitePiece(piece: Piece) {
        whitePieces.add(piece.type.toInt())
        whiteScore += piece.getScore()
    }

    fun addBlackPiece(piece: Piece) {
        blackPieces.add(piece.type.toInt() - 6)
        blackScore += piece.getScore()
    }

    fun reset() {
        whitePieces.clear()
        blackPieces.clear()
        whiteScore = 0
        blackScore = 0
    }

    private fun getDifference(): Pair<IntArray, IntArray> {
        val first = whitePieces.toMutableList()
        val second = blackPieces.toMutableList()

        for (element in first) {
            val index = second.indexOf(element)

            if (index != -1) {
                first.remove(element)
                second.removeAt(index)
            }
        }

        first.sortDescending()
        second.sortDescending()

        return Pair(first.toIntArray(), second.toIntArray())
    }
}
