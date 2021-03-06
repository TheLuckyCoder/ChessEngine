package net.theluckycoder.chess.utils

import net.theluckycoder.chess.model.Piece

class CapturedPieces(
    pieces: List<Piece>,
) {

    private val piecesPartition = pieces.partition { it.isWhite }
    private val whitePieces = piecesPartition.first
    private val blackPieces = piecesPartition.second

    private val absoluteWhiteScore = whitePieces.sumBy { it.score }
    private val absoluteBlackScore = blackPieces.sumBy { it.score }

    fun getDifference(): Pair<List<Piece>, List<Piece>> {
        val white = whitePieces.toMutableList()
        val black = blackPieces.toMutableList()

        for (element in whitePieces) {
            val index = black.indexOf(element)

            if (index != -1) {
                white.remove(element)
                black.removeAt(index)
            }
        }

        white.sortByDescending { it.type }
        black.sortByDescending { it.type }

        return white to black
    }
}
