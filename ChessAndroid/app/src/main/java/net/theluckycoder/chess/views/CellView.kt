package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import net.theluckycoder.chess.ChessActivity
import net.theluckycoder.chess.Pos

@SuppressLint("ViewConstructor")
class CellView(
    private val activity: ChessActivity,
    isWhiteBackground: Boolean,
    val pos: Pos,
    private val listener: CustomView.ClickListener
) : CustomView(activity) {

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
    private val possibleStatePaint = Paint().apply {
        color = Color.parseColor("#4caf50")
    }

    var state = State.NONE
        set(value) {
            field = value
            invalidate()
        }
    var lastMoved = false
        set(value) {
            field = value
            invalidate()
        }

    override fun onDraw(canvas: Canvas) {
        canvas.drawColor(backgroundColor)

        when (state) {
            State.SELECTED -> canvas.drawColor(Color.parseColor("#ef4fc3f7"))
            State.POSSIBLE -> if (activity.pieces[pos] == null) {
                canvas.drawCircle(width / 2f, height / 2f, width / 6.5f, possibleStatePaint)
            } else {
                canvas.drawPaint(possibleStatePaint)
            }
            else -> if (lastMoved) canvas.drawColor(Color.parseColor("#99fbc02d"))
        }
    }
}
