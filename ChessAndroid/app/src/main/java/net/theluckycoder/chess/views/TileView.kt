package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.graphics.Canvas
import android.graphics.Paint
import net.theluckycoder.chess.ChessActivity
import net.theluckycoder.chess.Pos

@SuppressLint("ViewConstructor")
class TileView(
    private val activity: ChessActivity,
    private val isWhiteBackground: Boolean,
    val pos: Pos,
    private val listener: ClickListener
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

    private val selectedStatePaint = Paint().apply {
        style = Paint.Style.STROKE
    }
    private val possibleStatePaint = Paint()

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

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        selectedStatePaint.strokeWidth = measuredHeight / 7f
    }

    override fun onDraw(canvas: Canvas) {
        val backgroundColor =
            if (isWhiteBackground) activity.preferences.whiteTileColor else activity.preferences.blackTileColor
        canvas.drawColor(backgroundColor)

        when (state) {
            State.SELECTED -> {
                selectedStatePaint.color = activity.preferences.selectedTileColor

                canvas.drawRect(0f, 0f, measuredWidth.toFloat(), measuredHeight.toFloat(), selectedStatePaint)
            }
            State.POSSIBLE -> {
                possibleStatePaint.color = activity.preferences.possibleTileColor
                if (activity.pieces[pos] == null) {
                    canvas.drawCircle(width / 2f, height / 2f, width / 6.5f, possibleStatePaint)
                } else {
                    canvas.drawPaint(possibleStatePaint)
                }
            }
            else -> if (lastMoved) canvas.drawColor(activity.preferences.lastMovedTileColor)
        }
    }
}
