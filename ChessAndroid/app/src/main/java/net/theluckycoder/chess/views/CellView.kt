package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import net.theluckycoder.chess.Pos

@SuppressLint("ViewConstructor")
class CellView(
    context: Context,
    isWhiteBackground: Boolean,
    val pos: Pos,
    private val listener: CustomView.ClickListener
) : CustomView(context) {

    init {
        setOnClickListener {
            listener.onClick(this)
        }
    }

    enum class State {
        NONE,
        SELECTED,
        POSSIBLE
    }

    private val backgroundColor = Color.parseColor(if (isWhiteBackground) "#eeeed2" else "#769656")
    private val statePaint = Paint().apply {
        color = Color.parseColor("#4caf50")
    }

    var state = State.NONE
        set(value) {
            field = value

            invalidate()
        }

    override fun onDraw(canvas: Canvas) {
        canvas.drawColor(backgroundColor)
        if (state == State.SELECTED) {
            canvas.drawColor(1727987712)
        } else if (state == State.POSSIBLE) {
            canvas.drawCircle(width / 2f, height / 2f, 21f, statePaint)
        }
    }
}
