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

    override fun onDraw(canvas: Canvas) {
        val backgroundColor =
            if (isWhiteBackground) activity.preferences.whiteTileColor else activity.preferences.blackTileColor
        canvas.drawColor(backgroundColor)

        when (state) {
            State.SELECTED -> canvas.drawColor(activity.preferences.selectedTileColor)
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
