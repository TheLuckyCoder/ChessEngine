package net.theluckycoder.chess

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.graphics.Point
import android.os.Bundle
import android.view.View
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.RelativeLayout
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_chess.*
import kotlinx.android.synthetic.main.dialog_restart.view.*
import net.theluckycoder.chess.views.CustomView
import net.theluckycoder.chess.views.PieceView
import net.theluckycoder.chess.views.TileView
import kotlin.concurrent.thread
import kotlin.random.Random

class ChessActivity : AppCompatActivity(), CustomView.ClickListener {

    companion object {
        private val pieceResources = arrayOf(
            0, R.drawable.w_pawn, R.drawable.w_knight,
            R.drawable.w_bishop, R.drawable.w_rook,
            R.drawable.w_queen, R.drawable.w_king,
            R.drawable.b_pawn, R.drawable.b_knight,
            R.drawable.b_bishop, R.drawable.b_rook,
            R.drawable.b_queen, R.drawable.b_king
        )
    }

    val preferences = Preferences(this)
    private val cells = HashMap<Pos, TileView>(64)
    private val capturedPieces = CapturedPieces()
    val pieces = HashMap<Pos, PieceView>(32)
    private var viewSize = 0

    private var selectedPos = Pos()
    private var showDebugInfo = false
    private var canMove = true
    private var restarting = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        initBoard(false)
        setContentView(R.layout.activity_chess)

        val point = Point()
        windowManager.defaultDisplay.getSize(point)

        viewSize = point.x / 8

        layout_board.layoutParams = RelativeLayout.LayoutParams(point.x, point.x)

        iv_settings.setOnClickListener {
            startActivity(Intent(this, SettingsActivity::class.java))
        }

        btn_restart_game.setOnClickListener {
            val view = View.inflate(this, R.layout.dialog_restart, null)

            AlertDialog.Builder(this)
                .setTitle(R.string.restart_game)
                .setView(view)
                .setPositiveButton(R.string.action_restart) { _, _ ->
                    val playerWhite = when (view.sp_color.selectedItemPosition) {
                        0 -> true
                        1 -> false
                        else -> Random.nextBoolean()
                    }

                    restartGame(playerWhite)
                }
                .setNegativeButton(android.R.string.no, null)
                .show()
        }

        if (preferences.firstStart) {
            preferences.firstStart = false
            // Set Default Settings
            preferences.settings = Settings(4, Runtime.getRuntime().availableProcessors() - 1, 200)
        }

        drawBoard()
        updatePieces()
    }

    override fun onStart() {
        super.onStart()

        cells.forEach {
            it.value.invalidate()
        }
        showDebugInfo = preferences.debugInfo
        Native.setSettings(preferences.settings)
    }

    override fun onClick(view: CustomView) {
        if (Native.isWorking()) return

        if (view is PieceView) {
            selectPiece(view)
        } else if (view is TileView && view.state == TileView.State.POSSIBLE) {
            movePiece(view)
        }
    }

    private fun drawBoard() {
        layout_board.removeAllViews()
        cells.clear()

        val isPlayerWhite = Native.isPlayerWhite()

        for (i in 0..7) {
            for (j in 0..7) {
                val pos = Pos(invertIf(!isPlayerWhite, i), invertIf(isPlayerWhite, j))
                val isWhite = (i + j) % 2 == 0

                val xSize = invertIf(!isPlayerWhite, pos.x.toInt()) * viewSize
                val ySize = invertIf(isPlayerWhite, pos.y.toInt()) * viewSize

                val cellView = TileView(this, isWhite, pos, this).apply {
                    layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                    x = xSize.toFloat()
                    y = ySize.toFloat()
                }

                cells[pos] = cellView
                layout_board.addView(cellView)
            }
        }
    }

    private fun clearCells(clearMoved: Boolean = false) {
        cells.forEach {
            if (clearMoved && it.value.lastMoved)
                it.value.lastMoved = false
            it.value.state = TileView.State.NONE
        }
    }

    private fun setKingInChess(white: Boolean) {
        val drawable = if (white) R.drawable.w_king else R.drawable.b_king

        pieces.forEach {
            if (it.value.res == drawable)
                it.value.isInChess = true
        }
    }

    private fun movePiece(view: TileView) {
        if (!selectedPos.isValid || !canMove) return

        val startPos = selectedPos
        Native.movePiece(selectedPos, view.pos)
        clearCells(true)
        selectedPos = Pos()

        cells[startPos]?.lastMoved = true
        cells[view.pos]?.lastMoved = true

        pb_loading.visibility = View.VISIBLE
    }

    private fun selectPiece(view: PieceView) {
        clearCells()

        val isPlayerWhite = Native.isPlayerWhite()
        val x = view.x.toInt() / viewSize
        val y = view.y.toInt() / viewSize

        selectedPos = Pos(invertIf(!isPlayerWhite, x), invertIf(isPlayerWhite, y))
        cells[selectedPos]?.state = TileView.State.SELECTED
        updatePossibleMoves(selectedPos)
    }

    private fun updatePieces() {
        pieces.forEach {
            layout_board.removeView(it.value)
        }
        pieces.clear()

        val isPlayerWhite = Native.isPlayerWhite()
        val newPieces = Native.getPieces()

        newPieces.forEach {
            val isWhite = it.type in 1..6
            val resource = pieceResources[it.type.toInt()]

            val xSize = invertIf(!isPlayerWhite, it.x.toInt()) * viewSize
            val ySize = invertIf(isPlayerWhite, it.y.toInt()) * viewSize

            val pieceView = PieceView(this, isWhite, resource, viewSize, this).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xSize.toFloat()
                y = ySize.toFloat()
            }

            pieces[Pos(it.x, it.y)] = pieceView
            layout_board.addView(pieceView)
        }
    }

    private fun updatePossibleMoves(pos: Pos) {
        cells[pos]?.state = TileView.State.SELECTED
        val possibleMoves = Native.getPossibleMoves(pos)

        possibleMoves?.forEach {
            cells[it]?.state = TileView.State.POSSIBLE
        }
    }

    private fun updateState(state: Int) {
        Native.enableStats(showDebugInfo)
        if (showDebugInfo) {
            tv_debug.visibility = View.VISIBLE
            tv_debug.text = getString(R.string.stats, Native.getStats(), Native.getBoardValue())
        } else {
            tv_debug.visibility = View.GONE
        }

        pb_loading.visibility = if (Native.isWorking()) View.VISIBLE else View.INVISIBLE

        if (state in 1..3) {
            canMove = false
            pb_loading.visibility = View.INVISIBLE

            val message = when (state) {
                1 -> "White has won!"
                2 -> "Black has won!"
                3 -> "Draw"
                else -> null
            }

            AlertDialog.Builder(this)
                .setTitle(message)
                .setPositiveButton(android.R.string.ok, null)
                .show()
        }

        if (state == 2 || state == 4) {
            setKingInChess(true)
        } else if (state == 1 || state == 5) {
            setKingInChess(false)
        } else {
            pieces.forEach {
                if (it.value.isInChess)
                    it.value.isInChess = false
            }
        }
    }

    @Suppress("unused")
    private fun callback(gameState: Int, shouldRedraw: Boolean, moves: Array<PosPair>) = runOnUiThread {
        clearCells(true)

        val isPlayerWhite = Native.isPlayerWhite()

        moves.forEach {
            val startPos = Pos(it.startX, it.startY)
            val destPos = Pos(it.destX, it.destY)

            if (startPos.isValid && destPos.isValid) {
                val pieceView = pieces.remove(startPos)!!

                val xPos = invertIf(!isPlayerWhite, it.destX.toInt()) * viewSize
                val yPos = invertIf(isPlayerWhite, it.destY.toInt()) * viewSize

                pieceView.animate()
                    .x(xPos.toFloat())
                    .y(yPos.toFloat())
                    .start()

                pieces[destPos]?.let { destView ->
                    // Remove the Destination Piece
                    layout_board.removeView(destView)
                }

                pieces[destPos] = pieceView

                cells[startPos]?.lastMoved = true
                cells[destPos]?.lastMoved = true
            }
        }

        if (shouldRedraw)
            updatePieces()

        updateState(gameState)
    }

    private fun restartGame(isPlayerWhite: Boolean) {
        if (restarting) return
        restarting = true

        val restart = {
            initBoard(true, isPlayerWhite)
            drawBoard()
            updatePieces()
            clearCells(true)
            updateState(0)
            capturedPieces.reset()
            canMove = true
            restarting = false
            tv_debug.text = null
        }

        if (Native.isWorking()) {
            thread {
                while (Native.isWorking()) {
                    Thread.sleep(200)
                }
                runOnUiThread(restart)
            }
        } else restart()
    }

    // Saving/Loading

    private fun loadMoves() {
        val editText = EditText(this)

        AlertDialog.Builder(this)
            .setTitle("Load Moves")
            .setView(editText)
            .setPositiveButton("Load") { _, _ ->
                Native.loadMoves(editText.text?.toString().orEmpty())
            }
            .show()
    }

    private fun saveMoves() {
        val moves = Native.saveMoves()

        AlertDialog.Builder(this)
            .setTitle("Moves")
            .setMessage(moves)
            .setNegativeButton(android.R.string.cancel, null)
            .setNeutralButton("Copy") { _, _ ->
                val clipboardManager = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager

                val clip = ClipData.newPlainText("label", moves)
                clipboardManager.primaryClip = clip
            }
            .show()
    }

    private fun invertIf(invert: Boolean, i: Int) = if (invert) 7 - i else i

    private external fun initBoard(restartGame: Boolean, isPlayerWhite: Boolean = true)
}
