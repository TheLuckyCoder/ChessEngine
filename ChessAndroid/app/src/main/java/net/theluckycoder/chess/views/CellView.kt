package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
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
    private var stateColor = 0

    var state = State.NONE
        set(value) {
            field = value

            stateColor = when (state) {
                State.NONE -> 0
                State.SELECTED -> 1727987712
                State.POSSIBLE -> 1711328256
            }

            invalidate()
        }

    override fun onDraw(canvas: Canvas) {
        canvas.drawColor(backgroundColor)
        canvas.drawColor(stateColor)
    }
}
