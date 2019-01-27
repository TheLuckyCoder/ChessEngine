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
import android.view.View
import android.widget.*
import net.theluckycoder.chess.views.CellView
import net.theluckycoder.chess.views.CustomView
import net.theluckycoder.chess.views.PieceView
import kotlin.concurrent.thread


class MainActivity : Activity(), CustomView.ClickListener {

    companion object {
        init {
            System.loadLibrary("chess")
        }

        var viewSize = 0
    }

    private lateinit var frameLayout: FrameLayout
    private lateinit var pbLoading: ProgressBar
    private lateinit var tvBoards: TextView
    private lateinit var tvState: TextView
    private val cells = HashMap<Pos, CellView>(64)
    val pieces = HashMap<Pos, PieceView>(32)
    private val isPlayerWhite = Native.isPlayerWhite()
    private var selectedPos = Pos()
    private var canMove = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        initBoard(false)
        setContentView(R.layout.activity_main)

        val display = windowManager.defaultDisplay
        val point = Point()
        display.getSize(point)

        viewSize = point.x / 8

        frameLayout = findViewById(R.id.layout_board)
        frameLayout.layoutParams = LinearLayout.LayoutParams(point.x, point.x)
        pbLoading = findViewById(R.id.pb_loading)
        tvBoards = findViewById(R.id.tv_boards)
        tvState = findViewById(R.id.tv_state)

        for (i in 0..7) {
            for (j in 0..7) {
                val pos = Pos(i, 7 - j)
                val isWhite = ((pos.x + pos.y) % 2 == 1) == isPlayerWhite

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
            R.id.action_load_json -> loadJson()
            R.id.action_load_moves -> loadMoves()
            R.id.action_save_json -> saveJson()
            R.id.action_save_moves -> saveMoves()
            R.id.action_restart -> {
                thread {
                    while (Native.isWorking()) {
                        Thread.sleep(200)
                    }
                    runOnUiThread {
                        initBoard(true)
                        updatePieces()
                        updateState(0)
                        canMove = true
                    }
                }
            }
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
        val boardValue = Native.getCurrentBoardEvaluation()
        val evaluatedBoards = Native.getNumberOfEvaluatedBoards()
        tvBoards.text = getString(R.string.board_state, boardValue, evaluatedBoards)

        if (evaluatedBoards.toString().contains("69", true))
            tvBoards.append("\tNice.")

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

    // Saving/Loading

    private fun loadJson() {
        val editText = EditText(this)

        AlertDialog.Builder(this)
            .setTitle("Load Json")
            .setView(editText)
            .setPositiveButton("Load") { _, _ ->
                Native.loadFromJson(editText.text?.toString().orEmpty())
                updatePieces()
            }
            .show()
    }

    private fun saveJson() {
        val json = Native.saveToJson()

        AlertDialog.Builder(this)
            .setTitle("Json")
            .setMessage(json)
            .setNegativeButton(android.R.string.cancel, null)
            .setNeutralButton("Copy") { _, _ ->
                val clipboardManager = getSystemService(Context.CLIPBOARD_SERVICE) as ClipboardManager

                val clip = ClipData.newPlainText("label", json)
                clipboardManager.primaryClip = clip
            }
            .show()
    }

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
