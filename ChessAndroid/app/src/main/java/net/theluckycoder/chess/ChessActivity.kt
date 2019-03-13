package net.theluckycoder.chess

import android.app.AlertDialog
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.graphics.Point
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.ProgressBar
import android.widget.RelativeLayout
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import net.theluckycoder.chess.views.CustomView
import net.theluckycoder.chess.views.PieceView
import net.theluckycoder.chess.views.TileView
import kotlin.concurrent.thread

class ChessActivity : AppCompatActivity(), CustomView.ClickListener {

    companion object {
        var viewSize = 0
    }

    private lateinit var frameLayout: FrameLayout
    private lateinit var pbLoading: ProgressBar
    private lateinit var tvDebug: TextView
    private val cells = HashMap<Pos, TileView>(64)
    val preferences = Preferences(this)
    val pieces = HashMap<Pos, PieceView>(32)
    private var selectedPos = Pos()
    private var showDebugInfo = false
    private var canMove = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        initBoard(false)
        setContentView(R.layout.activity_chess)

        val display = windowManager.defaultDisplay
        val point = Point()
        display.getSize(point)

        viewSize = point.x / 8

        frameLayout = findViewById(R.id.layout_board)
        frameLayout.layoutParams = RelativeLayout.LayoutParams(point.x, point.x)
        pbLoading = findViewById(R.id.pb_loading)
        tvDebug = findViewById(R.id.tv_debug)

        findViewById<ImageView>(R.id.iv_settings).setOnClickListener {
            startActivity(Intent(this, SettingsActivity::class.java))
        }

        findViewById<Button>(R.id.btn_restart_game).setOnClickListener {
            AlertDialog.Builder(this)
                .setTitle("Restart Game")
                .setMessage("Are you sure you wish to continue?")
                .setPositiveButton(android.R.string.yes) { _, _ ->
                    restartGame()
                }
                .setNegativeButton(android.R.string.no, null)
                .show()
        }

        if (preferences.firstStart) {
            preferences.firstStart = false
            // Set Default Settings
            preferences.settings = Settings(4, Runtime.getRuntime().availableProcessors() - 1, 200)
        }

        val isPlayerWhite = Native.isPlayerWhite()

        for (i in 0..7) {
            for (j in 0..7) {
                val pos = Pos(i, 7 - j)
                val isWhite = ((pos.x + pos.y) % 2 == 1) == isPlayerWhite

                val xSize = pos.x * viewSize
                val ySize = (if (isPlayerWhite) 7 - pos.y else pos.y.toInt()) * viewSize

                val cellView = TileView(this, isWhite, pos, this).apply {
                    layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                    x = xSize.toFloat()
                    y = ySize.toFloat()
                }

                cells[pos] = cellView
                frameLayout.addView(cellView)
            }
        }

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

    //------------------------------------------------------------------------

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

        pbLoading.visibility = View.VISIBLE
    }

    private fun selectPiece(view: PieceView) {
        clearCells()

        selectedPos = Pos(view.x.toInt() / viewSize, 7 - (view.y.toInt() / viewSize))
        cells[selectedPos]?.state = TileView.State.SELECTED
        updatePossibleMoves(selectedPos)
    }

    private fun updatePieces() {
        pieces.forEach {
            frameLayout.removeView(it.value)
        }
        pieces.clear()

        val newPieces = Native.getPieces()

        val pieceResources = arrayOf(
            0, R.drawable.w_pawn, R.drawable.w_knight, R.drawable.w_bishop,
            R.drawable.w_rook, R.drawable.w_queen, R.drawable.w_king,
            R.drawable.b_pawn, R.drawable.b_knight, R.drawable.b_bishop,
            R.drawable.b_rook, R.drawable.b_queen, R.drawable.b_king
        )

        newPieces.forEach {
            val isWhite = it.type in 1..6
            val resource = pieceResources[it.type.toInt()]

            val xSize = it.x * viewSize
            val ySize = (7 - it.y) * viewSize

            val pieceView = PieceView(this, isWhite, resource, this).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xSize.toFloat()
                y = ySize.toFloat()
            }

            pieces[Pos(it.x, it.y)] = pieceView
            frameLayout.addView(pieceView)
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
            tvDebug.visibility = View.VISIBLE
            tvDebug.text = getString(R.string.stats, Native.getStats(), Native.getBoardValue())
        } else {
            tvDebug.visibility = View.GONE
        }

        pbLoading.visibility = if (Native.isWorking()) View.VISIBLE else View.INVISIBLE

        if (state in 1..3) {
            canMove = false
            val message = when (state) {
                1 -> "White has won!"
                2 -> "Black has won!"
                3 -> "Draw"
                else -> null
            }

            AlertDialog.Builder(this)
                .setTitle("Game Over!")
                .setPositiveButton(android.R.string.ok, null)
                .setMessage(message)
                .show()
        }

        if (state == 1 || state == 4) {
            setKingInChess(true)
        } else if (state == 2 || state == 5) {
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

                cells[startPos]?.lastMoved = true
                cells[destPos]?.lastMoved = true
            }
        }

        if (shouldRedraw)
            updatePieces()

        updateState(gameState)
    }

    private fun restartGame() {
        thread {
            while (Native.isWorking()) {
                Thread.sleep(200)
            }
            runOnUiThread {
                initBoard(true)
                updatePieces()
                clearCells(true)
                updateState(0)
                canMove = true
            }
        }
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

    private external fun initBoard(restartGame: Boolean)
}
