package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.graphics.Bitmap
import android.graphics.BlurMaskFilter
import android.graphics.Canvas
import android.graphics.Paint
import net.theluckycoder.chess.ChessActivity
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.PieceResourceManager

@SuppressLint("ViewConstructor")
class PieceView(
    private val activity: ChessActivity,
    isWhite: Boolean,
    val res: Int,
    private val listener: ClickListener
) : CustomView(activity) {

    private val bitmap: Bitmap = PieceResourceManager.getBitmap(res)

    init {
        if (isWhite == Native.isPlayerWhite()) {
            setOnClickListener {
                listener.onClick(this)
            }
        }
    }

    private val redBlurPaint = Paint().apply {
        maskFilter = BlurMaskFilter(10f, BlurMaskFilter.Blur.NORMAL)
    }
    var isInChess = false
        set(value) {
            field = value
            invalidate()
        }

    override fun onDraw(canvas: Canvas) {
        if (isInChess) {
            redBlurPaint.color = activity.preferences.kingInChessColor
            canvas.drawCircle(width / 2f, height / 2f, width / 2.5f, redBlurPaint)
        }
        canvas.drawBitmap(bitmap, 0f, 0f, null)
    }
}
