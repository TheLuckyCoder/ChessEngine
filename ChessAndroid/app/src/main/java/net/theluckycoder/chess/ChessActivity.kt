package net.theluckycoder.chess

import android.app.Activity
import android.app.AlertDialog
import android.content.ClipData
import android.content.ClipboardManager
import android.content.Context
import android.graphics.Point
import android.os.Bundle
import android.os.Debug
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.widget.*
import net.theluckycoder.chess.views.CellView
import net.theluckycoder.chess.views.CustomView
import net.theluckycoder.chess.views.PieceView
import kotlin.concurrent.thread

class ChessActivity : Activity(), CustomView.ClickListener {

    companion object {
        init {
            System.loadLibrary("chess")
        }

        var viewSize = 0
    }

    private lateinit var frameLayout: FrameLayout
    private lateinit var pbLoading: ProgressBar
    private lateinit var tvStats: TextView
    private lateinit var tvState: TextView
    private val cells = HashMap<Pos, CellView>(64)
    val pieces = HashMap<Pos, PieceView>(32)
    private var selectedPos = Pos()
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
        frameLayout.layoutParams = LinearLayout.LayoutParams(point.x, point.x)
        pbLoading = findViewById(R.id.pb_loading)
        tvStats = findViewById(R.id.tv_stats)
        tvState = findViewById(R.id.tv_state)

        val isPlayerWhite = Native.isPlayerWhite()

        for (i in 0..7) {
            for (j in 0..7) {
                val pos = Pos(i, 7 - j)
                val isWhite = ((pos.x + pos.y) % 2 == 1) == isPlayerWhite

                val xSize = pos.x * viewSize
                val ySize = (7 - pos.y) * viewSize

                val cellView = CellView(this, isWhite, pos, this).apply {
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

    override fun onClick(view: CustomView) {
        if (Native.isWorking()) return

        if (view is PieceView) {
            selectPiece(view)
        } else if (view is CellView && view.state == CellView.State.POSSIBLE) {
            movePiece(view)
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.main_menu, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onMenuItemSelected(featureId: Int, item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_redraw -> updatePieces()
            R.id.action_memory -> updateMemoryUsage()
            R.id.action_load_moves -> loadMoves()
            R.id.action_save_moves -> saveMoves()
            R.id.action_restart -> restartGame()
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
        if (!selectedPos.isValid || !canMove) return

        Native.movePiece(selectedPos, view.pos)
        clearCells()
        selectedPos = Pos()

        pbLoading.visibility = View.VISIBLE
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

        val pieceResources = arrayOf(
            0, R.drawable.w_pawn, R.drawable.w_knight, R.drawable.w_bishop,
            R.drawable.w_rook, R.drawable.w_queen, R.drawable.w_king,
            R.drawable.b_pawn, R.drawable.b_knight, R.drawable.b_bishop,
            R.drawable.b_rook, R.drawable.b_queen, R.drawable.b_king
        )

        newPieces.forEach {
            val isWhite = it.type in 1..6
            val resource = pieceResources[it.type.toInt()]

            val xx = it.x * viewSize
            val yy = (7 - it.y) * viewSize

            val pieceView = PieceView(this, isWhite, resource, this).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xx.toFloat()
                y = yy.toFloat()
            }

            pieces[Pos(it.x, it.y)] = pieceView
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

    private fun updateState(state: Int) {
        tvStats.text = getString(R.string.stats, Native.getStats(), Native.getBoardValue())

        tvState.text = when (state) {
            1 -> "White has won!"
            2 -> "Black has won!"
            3 -> "Draw"
            4 -> "White is in Chess!"
            5 -> "Black is in Chess!"
            else -> null
        }

        pbLoading.visibility = if (Native.isWorking()) View.VISIBLE else View.INVISIBLE

        if (state in 1..3) {
            canMove = false

            AlertDialog.Builder(this)
                .setTitle("Game Over!")
                .setPositiveButton(android.R.string.ok, null)
                .show()
        }
    }

    private fun updateMemoryUsage() {
        val heapUsed = Debug.getNativeHeapAllocatedSize() / 1048576
        val heapSize = Debug.getNativeHeapSize() / 1048576
        val text = getString(R.string.memory_usage, heapUsed, heapSize)
        tvStats.append(text)
    }

    @Suppress("unused")
    private fun callback(gameState: Int, shouldRedraw: Boolean, moves: Array<PosPair>) {
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

            updateState(gameState)
        }
    }

    private fun restartGame() {
        thread {
            while (Native.isWorking()) {
                Thread.sleep(200)
            }
            runOnUiThread {
                initBoard(true)
                updatePieces()
                clearCells()
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
