package net.theluckycoder.chess.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.*
import android.util.TypedValue
import net.theluckycoder.chess.model.BoardAppearance
import net.theluckycoder.chess.model.Move
import net.theluckycoder.chess.model.Pos

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
    private val possibleStatePaint = Paint().apply {
        style = Paint.Style.FILL
    }
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
    private var possiblePaths = emptyArray<Path>()

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
    var isPlayerWhite = true
    var isInCheck = false
        set(value) {
            if (field != value) {
                field = value
                invalidate()
            }
        }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec)

        val width = measuredWidth.toFloat()
        val height = measuredHeight.toFloat()

        selectedStatePaint.strokeWidth = measuredHeight / 7f

        possiblePaths = arrayOf(
            genTrianglePath(0f, 0f, width / 3, 0f, 0f, height / 3),
            genTrianglePath(width, 0f, width - width / 3, 0f, width, height / 3),
            genTrianglePath(0f, height, 0f, height - height / 3, width / 3, height),
            genTrianglePath(width, height, width, height - height / 3, width - width / 3, height)
        )
    }

    override fun onDraw(canvas: Canvas) {
        val width = measuredWidth.toFloat()
        val height = measuredHeight.toFloat()
        canvas.drawColor(tileBackgroundColor)

        if (isInCheck)
            canvas.drawCircle(width / 2f, height / 2f, width / 2.5f, checkPaint)

        when (state) {
            State.SELECTED -> canvas.drawRect(0f, 0f, width, height, selectedStatePaint)
            State.POSSIBLE -> {
                if (pieces[pos] == null) {
                    canvas.drawCircle(width / 2f, height / 2f, width / 6.5f, possibleStatePaint)
                } else {
                    for (i in 0 until 4)
                        canvas.drawPath(possiblePaths[i], possibleStatePaint)
                }
            }
            else -> if (lastMoved) canvas.drawColor(appearance.lastMoved)
        }

        if (appearance.showCoordinates) {
            val letterSize = letterPaint.textSize
            if ((isPlayerWhite && pos.y == 0) || (!isPlayerWhite && pos.y == 7)) {
                canvas.drawText(
                    ('A' + pos.x).toString(),
                    width - letterSize * 0.7f,
                    height,
                    letterPaint
                )
            }

            if ((isPlayerWhite && pos.x == 0) || (!isPlayerWhite && pos.x == 7))
                canvas.drawText(('1' + pos.y).toString(), 0f, letterSize, letterPaint)
        }
    }

    private fun genTrianglePath(
        x1: Float, y1: Float,
        x2: Float, y2: Float,
        x3: Float, y3: Float
    ) = Path().apply {
        moveTo(x1, y1)
        lineTo(x2, y2)
        lineTo(x3, y3)
        lineTo(x1, y1)
        close()
    }
}
