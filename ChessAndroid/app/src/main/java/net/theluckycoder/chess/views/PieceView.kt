package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import net.theluckycoder.chess.MainActivity

@SuppressLint("ViewConstructor")
class PieceView(
    context: Context,
    isWhite: Boolean,
    res: Int,
    private val listener: ClickListener
) : CustomView(context) {

    private val drawable: Bitmap

    init {
        val bitmap = BitmapFactory.decodeResource(context.resources, res)
        drawable = Bitmap.createScaledBitmap(bitmap, MainActivity.viewSize, MainActivity.viewSize, false)

        if (isWhite) {
            setOnClickListener {
                listener.onClick(this)
            }
        }
    }

    override fun onDraw(canvas: Canvas) {
        canvas.drawBitmap(drawable, 0f, 0f, null)
    }
}
