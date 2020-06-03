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

    private val redBlurPaint = Paint().apply {
        maskFilter = BlurMaskFilter(10f, BlurMaskFilter.Blur.NORMAL)
    }
    var isInCheck = false
        set(value) {
            field = value
            invalidate()
        }

    fun setCheckColor(value: Int) {
        if (value != redBlurPaint.color) {
            redBlurPaint.color = value
            invalidate()
        }
    }

    override fun onDraw(canvas: Canvas) {
        if (isInCheck)
            canvas.drawCircle(width / 2f, height / 2f, width / 2.5f, redBlurPaint)

        canvas.drawBitmap(bitmap, 0f, 0f, null)
    }
}
