package net.theluckycoder.chess

import android.app.Activity
import android.app.AlertDialog
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.graphics.Point
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.widget.EditText
import android.widget.FrameLayout
import net.theluckycoder.chess.views.CellView
import net.theluckycoder.chess.views.CustomView
import net.theluckycoder.chess.views.PieceView

class MainActivity : Activity(), CustomView.ClickListener {

    companion object {
        init {
            System.loadLibrary("chess")
        }

        var viewSize = 0
    }

    private lateinit var frameLayout: FrameLayout
    private val cells = HashMap<Pos, CellView>(64)
    private val pieces = HashMap<Pos, PieceView>(32)
    private val isWhiteAtBottom = Native.isWhiteAtBottom()
    private var selectedPos = Pos()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        initBoard()

        frameLayout = FrameLayout(this)
        addContentView(
            frameLayout, FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.MATCH_PARENT
            )
        )

        val display = windowManager.defaultDisplay
        val point = Point()
        display.getSize(point)

        viewSize = point.x / 8

        for (i in 0..7) {
            for (j in 0..7) {
                val pos = Pos(i, 7 - j)
                val isWhite = ((pos.x + pos.y) % 2 == 1) == isWhiteAtBottom

                val xx = pos.x * viewSize
                val yy = (7 - pos.y) * viewSize

                val cellView = CellView(this, isWhite, pos, this).apply {
                    layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                    x = xx.toFloat()
                    y = yy.toFloat()
                }

                cells[pos] = cellView
                frameLayout.addView(cellView)
            }
        }

        updatePieces()
    }

    override fun onClick(view: CustomView) {
        if (Native.isWorking()) return

        if (view is PieceView) {
            selectPiece(view)
        } else if (view is CellView) {
            if (view.state == CellView.State.POSSIBLE) {
                movePiece(view)
            }
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onMenuItemSelected(featureId: Int, item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_load -> loadJson()
            R.id.action_save -> saveJson()
        }
        return super.onMenuItemSelected(featureId, item)
    }

    //------------------------------------------------------------------------

    private fun clearCells() {
        cells.forEach {
            it.value.state = CellView.State.NONE
        }
    }

    private fun movePiece(view: CellView) {
        if (!selectedPos.isValid) return

        Native.movePiece(selectedPos, view.pos)
        clearCells()
        selectedPos = Pos()
    }

    private fun selectPiece(view: PieceView) {
        clearCells()

        selectedPos = Pos(view.x.toInt() / viewSize, 7 - (view.y.toInt() / viewSize))
        cells[selectedPos]?.state = CellView.State.SELECTED
        updatePossibleMoves(selectedPos)
    }

    private fun updatePieces() {
        pieces.forEach {
            frameLayout.removeView(it.value)
        }
        pieces.clear()

        val newPieces = Native.getPieces()

        newPieces.forEach {
            var isWhite = false
            val res = when (it.type) {
                PieceType.PAWN -> {
                    isWhite = true
                    R.drawable.w_pawn
                }
                PieceType.KNIGHT -> {
                    isWhite = true
                    R.drawable.w_knight
                }
                PieceType.BISHOP -> {
                    isWhite = true
                    R.drawable.w_bishop
                }
                PieceType.ROOK -> {
                    isWhite = true
                    R.drawable.w_rook
                }
                PieceType.QUEEN -> {
                    isWhite = true
                    R.drawable.w_queen
                }
                PieceType.KING -> {
                    isWhite = true
                    R.drawable.w_king
                }
                PieceType.PAWN_BLACK -> R.drawable.b_pawn
                PieceType.KNIGHT_BLACK -> R.drawable.b_knight
                PieceType.BISHOP_BLACK -> R.drawable.b_bishop
                PieceType.ROOK_BLACK -> R.drawable.b_rook
                PieceType.QUEEN_BLACK -> R.drawable.b_queen
                PieceType.KING_BLACK -> R.drawable.b_king
                else -> throw IllegalArgumentException("PieceType ${it.type} is invalid!")
            }

            val xx = it.x * viewSize
            val yy = (7 - it.y) * viewSize

            val pieceView = PieceView(this, isWhite, res, this).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xx.toFloat()
                y = yy.toFloat()
            }

            this.pieces[Pos(it.x, it.y)] = pieceView
            frameLayout.addView(pieceView)
        }
    }

    private fun updatePossibleMoves(pos: Pos) {
        cells[pos]?.state = CellView.State.SELECTED
        val possibleMoves = Native.getPossibleMoves(pos)

        possibleMoves?.forEach {
            cells[it]?.state = CellView.State.POSSIBLE
        }
    }

    private fun showGameOverDialog(state: Byte) {
        val message = when (state) {
            1.toByte() -> "White has won!"
            2.toByte() -> "Black has won!"
            else -> "Draw"
        }

        AlertDialog.Builder(this)
            .setTitle("Game Over!")
            .setMessage(message)
            .setCancelable(false)
            .setPositiveButton(android.R.string.ok) { _, _ ->
                finish()
            }
            .show()
    }

    @Suppress("unused")
    private fun callback(gameState: Byte, shouldRedraw: Boolean, moves: Array<PosPair>) {
        runOnUiThread {
            moves.forEach {
                val startPos = Pos(it.startX, it.startY)
                val destPos = Pos(it.destX, it.destY)

                if (startPos.isValid && destPos.isValid) {
                    val pieceView = pieces.remove(startPos)!!

                    val xx = it.destX * viewSize
                    val yy = (7 - it.destY) * viewSize

                    pieceView.animate()
                        .x(xx.toFloat())
                        .y(yy.toFloat())
                        .start()

                    pieces[destPos]?.let { destView ->
                        frameLayout.removeView(destView)
                    }

                    pieces[destPos] = pieceView
                }
            }

            if (shouldRedraw)
                updatePieces()

            if (gameState != 0.toByte())
                showGameOverDialog(gameState)
        }
    }

    private fun loadJson() {
        val editText = EditText(this)

        AlertDialog.Builder(this)
            .setTitle("Load Json")
            .setView(editText)
            .setPositiveButton("Load") { _, _ ->
                Native.loadFromJson(editText.toString())
                updatePieces()
            }
            .show()
    }

    private fun saveJson() {
        val json = Native.saveToJson()
        AlertDialog.Builder(this)
            .setTitle("Json")
            .setMessage(json)
            .setPositiveButton(android.R.string.ok, null)
            .setNeutralButton("Copy") { _, _ ->
                val clipboardManager = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager

                val clip = ClipData.newPlainText("label", json)
                clipboardManager.primaryClip = clip
            }
            .show()
    }

    private external fun initBoard()
}
