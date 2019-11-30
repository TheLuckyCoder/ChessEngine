package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.graphics.Bitmap
import android.graphics.BlurMaskFilter
import android.graphics.Canvas
import android.graphics.Paint
import net.theluckycoder.chess.ChessActivity
import net.theluckycoder.chess.utils.PieceResourceManager
import net.theluckycoder.chess.Pos

@SuppressLint("ViewConstructor")
class PieceView(
    private val activity: ChessActivity,
    clickable: Boolean,
    val res: Int,
    var pos: Pos,
    private val listener: ClickListener
) : CustomView(activity) {

    private val bitmap: Bitmap = PieceResourceManager.getBitmap(res)

    init {
        if (clickable) {
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

    override fun onDraw(canvas: Canvas) {
        if (isInCheck) {
            redBlurPaint.color = activity.preferences.kingInChessColor
            canvas.drawCircle(width / 2f, height / 2f, width / 2.5f, redBlurPaint)
        }
        canvas.drawBitmap(bitmap, 0f, 0f, null)
    }
}
