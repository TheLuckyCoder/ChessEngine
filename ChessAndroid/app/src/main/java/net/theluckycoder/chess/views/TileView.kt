package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.BlurMaskFilter
import android.graphics.Canvas
import android.graphics.Paint
import android.util.TypedValue
import net.theluckycoder.chess.model.Pos
import net.theluckycoder.chess.model.BoardAppearance
import net.theluckycoder.chess.model.Move

@SuppressLint("ViewConstructor")
class TileView(
    context: Context,
    private val isWhiteBackground: Boolean,
    val pos: Pos,
    private val pieces: HashMap<Pos, PieceView>,
    listener: SimpleClickListener
) : CustomView(context) {

    enum class State {
        NONE,
        SELECTED,
        POSSIBLE
    }

    init {
        setOnClickListener {
            listener.onClick(this)
        }
    }

    // Paint objects and Colors needed for drawing
    private var tileBackgroundColor = 0
    private val selectedStatePaint = Paint().apply {
        style = Paint.Style.STROKE
    }
    private val possibleStatePaint = Paint()
    private val letterPaint = Paint().apply {
        textSize = TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_SP,
            15f,
            context.resources.displayMetrics
        )
    }
    private val checkPaint = Paint().apply {
        maskFilter = BlurMaskFilter(10f, BlurMaskFilter.Blur.NORMAL)
    }

    // Public Fields
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
    val storedMoves = ArrayList<Move>(5)
    var appearance = BoardAppearance(0, 0, 0, 0, 0, false)
        set(value) {
            if (field != value) {
                field = value

                selectedStatePaint.color = value.lastMoved
                possibleStatePaint.color = value.possible
                checkPaint.color = value.kingInCheck

                tileBackgroundColor = if (isWhiteBackground) {
                    letterPaint.color = value.blackTile
                    value.whiteTile
                } else {
                    letterPaint.color = value.whiteTile
                    value.blackTile
                }

                invalidate()
            }
        }
    var isInCheck = false
        set(value) {
            field = value
            invalidate()
        }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)
        selectedStatePaint.strokeWidth = measuredHeight / 7f
    }

    override fun onDraw(canvas: Canvas) {
        canvas.drawColor(tileBackgroundColor)

        if (isInCheck)
            canvas.drawCircle(width / 2f, height / 2f, width / 2.5f, checkPaint)

        when (state) {
            State.SELECTED -> {
                canvas.drawRect(
                    0f,
                    0f,
                    measuredWidth.toFloat(),
                    measuredHeight.toFloat(),
                    selectedStatePaint
                )
            }
            State.POSSIBLE -> {
                if (pieces[pos] == null) {
                    canvas.drawCircle(width / 2f, height / 2f, width / 6.5f, possibleStatePaint)
                } else {
                    canvas.drawPaint(possibleStatePaint)
                }
            }
            else -> if (lastMoved) canvas.drawColor(appearance.lastMoved)
        }

        if (appearance.showCoordinates) {
            val letterSize = letterPaint.textSize
            if (pos.y == 0) {
                canvas.drawText(
                    ('A' + pos.x).toString(),
                    measuredWidth.toFloat() - letterSize * 0.75f,
                    measuredHeight.toFloat(),
                    letterPaint
                )
            }

            if (pos.x == 0)
                canvas.drawText(('1' + pos.y).toString(), 0f, letterSize, letterPaint)
        }
    }
}
