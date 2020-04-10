package net.theluckycoder.chess

import android.content.Intent
import android.graphics.Point
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.view.animation.AccelerateDecelerateInterpolator
import android.widget.FrameLayout
import android.widget.RelativeLayout
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_chess.*
import kotlinx.android.synthetic.main.dialog_restart.view.*
import net.theluckycoder.chess.utils.AppPreferences
import net.theluckycoder.chess.utils.CapturedPieces
import net.theluckycoder.chess.utils.PieceResourceManager
import net.theluckycoder.chess.views.CustomView
import net.theluckycoder.chess.views.PieceView
import net.theluckycoder.chess.views.TileView
import kotlin.concurrent.thread
import kotlin.random.Random

class ChessActivity : AppCompatActivity(), CustomView.ClickListener, GameManager.OnEventListener {

    private val gameManager by lazy(LazyThreadSafetyMode.NONE) { GameManager(this, this) }
    private val tiles = HashMap<Pos, TileView>(64)
    private val capturedPieces = CapturedPieces()
    private var viewSize = 0

    val preferences = AppPreferences(this)
    val pieces = HashMap<Pos, PieceView>(32)

    private var selectedPos = Pos()
    private var canMove = true
    private var restarting = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_chess)

        val point = Point()
        windowManager.defaultDisplay.getSize(point)

        viewSize = point.x / 8
        PieceResourceManager.init(this, viewSize)

        layout_board.layoutParams = RelativeLayout.LayoutParams(point.x, point.x)

        iv_undo.setOnClickListener {
            Native.undoMoves()
        }

        iv_settings.setOnClickListener {
            startActivity(Intent(this, SettingsActivity::class.java))
        }

        iv_settings.setOnLongClickListener {
            val isPlayerWhite = Native.isPlayerWhite()
            redrawBoard(isPlayerWhite)
            redrawPieces(Native.getPieces().toList(), isPlayerWhite)

            true
        }

        btn_restart_game.setOnClickListener {
            val view = View.inflate(this, R.layout.dialog_restart, null)

            AlertDialog.Builder(this)
                .setTitle(R.string.new_game)
                .setView(view)
                .setPositiveButton(R.string.action_start) { _, _ ->
                    val playerWhite = when (view.sp_side.selectedItemPosition) {
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
            preferences.settings =
                Settings(4, Runtime.getRuntime().availableProcessors() - 1, 100, true)
        }

        gameManager.initBoard(false)
    }

    override fun onStart() {
        super.onStart()

        tiles.forEach {
            it.value.invalidate()
        }
        gameManager.basicStatsEnabled = preferences.basicDebugInfo
        gameManager.advancedStatsEnabled = preferences.advancedDebugInfo
        gameManager.updateSettings(preferences.settings)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_main, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_force_move -> Native.forceMove()
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onClick(view: CustomView) {
        if (gameManager.isWorking) return

        if (view is PieceView) {
            selectPiece(view)
        } else if (view is TileView && view.state == TileView.State.POSSIBLE) {
            movePiece(view)
        }
    }

    private fun clearTiles(clearMoved: Boolean = false) {
        tiles.forEach {
            if (clearMoved && it.value.lastMoved)
                it.value.lastMoved = false
            it.value.state = TileView.State.NONE
            it.value.storedMove = 0
        }
    }

    private fun setKingInChess(white: Boolean) {
        val drawable = if (white) R.drawable.w_king else R.drawable.b_king

        pieces.forEach {
            if (it.value.res == drawable)
                it.value.isInCheck = true
        }
    }

    private fun movePiece(view: TileView) {
        if (!selectedPos.isValid || !canMove) return
        if (view.storedMove != 0L) {
            gameManager.makeMove(view.storedMove)
            clearTiles(true)
            selectedPos = Pos()

            pb_loading.visibility = View.VISIBLE
        }
    }

    private fun selectPiece(view: PieceView) {
        clearTiles()

        selectedPos = view.pos
        tiles[selectedPos]?.state = TileView.State.SELECTED

        val possibleMoves = gameManager.selectPiece(selectedPos)

        possibleMoves.forEach { move ->
            val to = (move ushr 15).toInt() and 0x3F

            tiles[Pos(to.toByte())]?.let {
                it.state = TileView.State.POSSIBLE
                it.storedMove = move
            }
        }
    }

    private fun updateState(state: State) {
        canMove = true

        if (gameManager.basicStatsEnabled) {
            tv_debug.visibility = View.VISIBLE

            val advancedStats =
                if (gameManager.advancedStatsEnabled) "\n" + Native.getAdvancedStats() else ""

            tv_debug.text = getString(
                R.string.basic_stats,
                Native.getSearchTime(),
                Native.getCurrentBoardValue(),
                Native.getBestMoveFound(),
                advancedStats
            )
        } else {
            tv_debug.visibility = View.GONE
        }

        val message = when (state) {
            State.WINNER_WHITE -> "White has won!"
            State.WINNER_BLACK -> "Black has won!"
            State.DRAW -> "Draw"
            else -> null
        }

        if (message != null) {
            canMove = false
            pb_loading.visibility = View.INVISIBLE

            AlertDialog.Builder(this)
                .setTitle(message)
                .setPositiveButton(android.R.string.ok, null)
                .show()
        } else {
            pb_loading.visibility = if (gameManager.isWorking) View.VISIBLE else View.INVISIBLE
        }

        if (state == State.WHITE_IN_CHESS || state == State.WINNER_BLACK) {
            setKingInChess(true)
        } else if (state == State.BLACK_IN_CHESS || state == State.WINNER_WHITE) {
            setKingInChess(false)
        } else {
            pieces.forEach {
                if (it.value.isInCheck)
                    it.value.isInCheck = false
            }
        }
    }

    private fun restartGame(isPlayerWhite: Boolean) {
        if (restarting) return
        restarting = true

        val restart = {
            gameManager.initBoard(true, isPlayerWhite)
            clearTiles(true)
            updateState(State.NONE)
            capturedPieces.reset()
            canMove = true
            restarting = false
            tv_debug.text = null
        }

        if (gameManager.isWorking) {
            thread {
                while (gameManager.isWorking)
                    Thread.sleep(200)

                runOnUiThread(restart)
            }
        } else restart()
    }

    override fun redrawBoard(isPlayerWhite: Boolean) {
        tiles.forEach {
            layout_board.removeView(it.value)
        }
        tiles.clear()

        for (i in 0 until 64) {
            val pos = Pos(i % 8, i / 8)
            val isWhite = (pos.x + pos.y) % 2 == 1

            val xSize = invertIf(!isPlayerWhite, pos.x) * viewSize
            val ySize = invertIf(isPlayerWhite, pos.y) * viewSize

            val tileView = TileView(this, isWhite, pos, this).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xSize.toFloat()
                y = ySize.toFloat()
            }

            tiles[pos] = tileView
            layout_board.addView(tileView)
        }
    }

    override fun redrawPieces(newPieces: List<Piece>, isPlayerWhite: Boolean) {
        pieces.forEach {
            layout_board.removeView(it.value)
        }
        pieces.clear()

        newPieces.forEach {
            val isWhite = it.type in 1..6
            val resource = PieceResourceManager.piecesResources[it.type.toInt() - 1]
            val clickable = isWhite == isPlayerWhite

            val pos = Pos(it.x, it.y)

            val xSize = invertIf(!isPlayerWhite, pos.x) * viewSize
            val ySize = invertIf(isPlayerWhite, pos.y) * viewSize

            val pieceView = PieceView(this, clickable, resource, pos, this).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xSize.toFloat()
                y = ySize.toFloat()
            }

            pieces[pos] = pieceView
            layout_board.addView(pieceView)
        }
    }

    override fun onMove(gameState: State) {
        clearTiles(true)
        updateState(gameState)
    }

    override fun onPieceMoved(startPos: Pos, destPos: Pos, isPlayerWhite: Boolean) {
        if (startPos.isValid && destPos.isValid) {
            val pieceView = pieces.remove(startPos)

            pieceView?.let {
                it.pos = destPos

                // Calculate the new View Position
                val xPos = invertIf(!isPlayerWhite, destPos.x) * viewSize
                val yPos = invertIf(isPlayerWhite, destPos.y) * viewSize

                it.animate()
                    .x(xPos.toFloat())
                    .y(yPos.toFloat())
                    .setDuration(250L)
                    .setInterpolator(AccelerateDecelerateInterpolator())
                    .start()
            }

            pieces[destPos]?.let { destView ->
                // Remove the Destination Piece
                layout_board.removeView(destView)

                val type = PieceResourceManager.piecesResources.indexOf(destView.res) + 1
                val piece = Piece(
                    invertIf(!isPlayerWhite, destView.pos.x),
                    invertIf(isPlayerWhite, destView.pos.y),
                    type.toByte()
                )

                if (isPlayerWhite)
                    capturedPieces.addBlackPiece(piece)
                else
                    capturedPieces.addWhitePiece(piece)
            }

            pieceView?.let {
                pieces[destPos] = it
            }

            tiles[startPos]?.lastMoved = true
            tiles[destPos]?.lastMoved = true
        }
    }

    private fun invertIf(invert: Boolean, i: Int) = if (invert) 7 - i else i
}
