package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.*
import net.theluckycoder.chess.utils.PieceResourceManager
import net.theluckycoder.chess.model.Pos

@SuppressLint("ViewConstructor")
class PieceView(
    context: Context,
    val pieceImage: Int,
    var pos: Pos,
    listener: SimpleClickListener?
) : CustomView(context) {

    private val bitmap: Bitmap = PieceResourceManager.getBitmap(pieceImage)

    init {
        if (listener != null) {
            setOnClickListener {
                listener.onClick(this)
            }
        }
    }

    override fun onDraw(canvas: Canvas) {
        canvas.drawBitmap(bitmap, 0f, 0f, null)
    }
}
