package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import net.theluckycoder.chess.ChessActivity
import net.theluckycoder.chess.Native

@SuppressLint("ViewConstructor")
class PieceView(
    context: Context,
    isWhite: Boolean,
    res: Int,
    private val listener: ClickListener
) : CustomView(context) {

    private val bitmap: Bitmap

    init {
        val decodedBitmap = BitmapFactory.decodeResource(context.resources, res)
        bitmap = Bitmap.createScaledBitmap(decodedBitmap, ChessActivity.viewSize, ChessActivity.viewSize, true)
        decodedBitmap.recycle()

        if (isWhite == Native.isPlayerWhite()) {
            setOnClickListener {
                listener.onClick(this)
            }
        }
    }

    private val blurPaint = Paint().apply {
        maskFilter = BlurMaskFilter(8, BlurMaskFilter.Blur.OUTER)
    }
    var isInChess = false
        set(value) {
            field = value
            invalidate()
        }

    override fun onDraw(canvas: Canvas) {
        canvas.drawBitmap(bitmap, 0f, 0f, null)
        if (isInChess)
            canvas.drawCircle(width / 2f, height / 2f, width / 3f, blurPaint)
    }
}
