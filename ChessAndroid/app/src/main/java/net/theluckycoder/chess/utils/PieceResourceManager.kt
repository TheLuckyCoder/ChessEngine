package net.theluckycoder.chess.utils

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.SparseArray
import net.theluckycoder.chess.R

object PieceResourceManager {

    val piecesResources = arrayOf(
        R.drawable.w_pawn,
        R.drawable.w_knight,
        R.drawable.w_bishop,
        R.drawable.w_rook,
        R.drawable.w_queen,
        R.drawable.w_king,
        R.drawable.b_pawn,
        R.drawable.b_knight,
        R.drawable.b_bishop,
        R.drawable.b_rook,
        R.drawable.b_queen,
        R.drawable.b_king
    )

    private val piecesBitmaps = SparseArray<Bitmap>(piecesResources.size)

    fun init(context: Context, size: Int) {
        val resources = context.resources

        piecesResources.forEach { res ->
            val decodedBitmap = BitmapFactory.decodeResource(resources, res)

            piecesBitmaps.put(
                res,
                Bitmap.createScaledBitmap(decodedBitmap, size, size, true)
            )

            decodedBitmap.recycle()
        }
    }

    fun getBitmap(resource: Int): Bitmap = piecesBitmaps[resource]
}