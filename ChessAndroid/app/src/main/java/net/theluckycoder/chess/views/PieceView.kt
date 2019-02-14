package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import net.theluckycoder.chess.ChessActivity

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

        if (isWhite) {
            setOnClickListener {
                listener.onClick(this)
            }
        }
    }

    override fun onDraw(canvas: Canvas) {
        canvas.drawBitmap(bitmap, 0f, 0f, null)
    }
}
