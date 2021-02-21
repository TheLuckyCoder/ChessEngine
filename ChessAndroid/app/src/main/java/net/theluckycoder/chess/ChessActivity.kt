package net.theluckycoder.chess

import android.content.ClipData
import android.content.ClipboardManager
import android.content.Intent
import android.content.res.Configuration
import android.graphics.drawable.ClipDrawable
import android.graphics.drawable.LayerDrawable
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.view.animation.AccelerateDecelerateInterpolator
import android.widget.FrameLayout
import android.widget.RelativeLayout
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.getSystemService
import net.theluckycoder.chess.databinding.ActivityChessBinding
import net.theluckycoder.chess.databinding.DialogImportFenBinding
import net.theluckycoder.chess.databinding.DialogNewGameBinding
import net.theluckycoder.chess.databinding.DialogPromotionBinding
import net.theluckycoder.chess.model.*
import net.theluckycoder.chess.utils.*
import net.theluckycoder.chess.views.CustomView
import net.theluckycoder.chess.views.PieceView
import net.theluckycoder.chess.views.TileView
import kotlin.concurrent.thread
import kotlin.random.Random

class ChessActivity : AppCompatActivity(),
    CustomView.SimpleClickListener,
    GameManager.OnEventListener {

    private val gameManager by lazy { GameManager(this, this) }
    private val tiles = HashMap<Pos, TileView>(64)
    private val pieces = HashMap<Pos, PieceView>(32)
    private val capturedPieces = CapturedPieces()
    private var viewSize = 0

    private val preferences = AppPreferences

    private var selectedPos = Pos()
    private var canMove = true
    private var restarting = false

    private lateinit var binding: ActivityChessBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityChessBinding.inflate(layoutInflater)
        setContentView(binding.root)

        val windowSize = getWindowSize()
        val orientation = resources.configuration.orientation

        viewSize = if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
            (windowSize.y - getActionAndStatusBarHeight()) / 8
        } else {
            windowSize.x / 8
        }

        PieceResourceManager.init(this, viewSize)

        binding.layoutBoard.layoutParams = RelativeLayout.LayoutParams(viewSize * 8, viewSize * 8)

        binding.ivUndo.setOnClickListener {
            Native.undoMoves()
        }

        binding.ivSettings.setOnClickListener {
            startActivity(Intent(this, SettingsActivity::class.java))
        }

        binding.ivSettings.setOnLongClickListener {
            val isPlayerWhite = Native.isPlayerWhite()
            redrawBoard(isPlayerWhite)
            redrawPieces(Native.getPieces().toList(), isPlayerWhite)

            true
        }

        binding.ivNewGame.setOnClickListener {
            showNewGameDialog()
        }

        if (preferences.firstStart) {
            preferences.firstStart = false

            // Set Default Settings
            val settings = EngineSettings.create(0, 0, 64, true)
            preferences.engineSettings = getDifficulty(DEFAULT_DIFFICULTY_LEVEL, settings)
            preferences.difficultyLevel = DEFAULT_DIFFICULTY_LEVEL
        }

        gameManager.initBoard()
    }

    override fun onStart() {
        super.onStart()

        with(gameManager) {
            val showDebugInfo = preferences.basicDebugInfo
            if (basicStatsEnabled != showDebugInfo) {
                basicStatsEnabled = showDebugInfo
                invalidateOptionsMenu()
            }
            advancedStatsEnabled = preferences.advancedDebugInfo
            updateSettings(preferences.engineSettings)
        }

        val boardAppearance = preferences.boardAppearance

        tiles.forEach {
            it.value.appearance = boardAppearance
        }
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_main, menu)
        return super.onCreateOptionsMenu(menu)
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.action_load_fen -> {
                val dialogBinding = DialogImportFenBinding.inflate(layoutInflater)

                val dialog = AlertDialog.Builder(this)
                    .setTitle(R.string.action_load_fen)
                    .setView(dialogBinding.root)
                    .create()

                dialogBinding.btnCancel.setOnClickListener { dialog.dismiss() }

                dialogBinding.btnLoad.setOnClickListener {
                    val fenPosition = dialogBinding.etFen.text?.toString()

                    if (fenPosition.isNullOrBlank()) {
                        dialogBinding.etFen.error = getString(R.string.error_empty_text_field)
                        return@setOnClickListener
                    }

                    val playerWhite = when (dialogBinding.tgSide.checkedButtonId) {
                        dialogBinding.btnSideWhite.id -> true
                        dialogBinding.btnSideBlack.id -> false
                        else -> Random.nextBoolean()
                    }

                    if (Native.loadFen(playerWhite, fenPosition)) {
                        gameManager.initBoard(playerWhite)
                        Native.loadFen(playerWhite, fenPosition)

                        Toast.makeText(this, R.string.fen_position_loaded, Toast.LENGTH_SHORT)
                            .show()
                        dialog.dismiss()
                    } else {
                        dialogBinding.etFen.error = getString(R.string.error_fen_position_invalid)
                        Toast.makeText(this, R.string.fen_position_error, Toast.LENGTH_LONG)
                            .show()
                    }
                }

                dialog.show()
            }
            R.id.action_export_fen -> {
                val fen = Native.getFen()

                AlertDialog.Builder(this)
                    .setTitle(R.string.fen_position_exported)
                    .setMessage(fen)
                    .setNeutralButton(android.R.string.copy) { _, _ ->
                        val clipboard = getSystemService<ClipboardManager>()
                        val clip = ClipData.newPlainText("FEN Position", fen)
                        clipboard!!.setPrimaryClip(clip)
                        Toast.makeText(this, R.string.fen_position_copied, Toast.LENGTH_SHORT)
                            .show()
                    }
                    .setNegativeButton(android.R.string.cancel, null)
                    .show()
            }
            R.id.action_force_move -> {
                Native.forceMove()
                showLoadingBar()
            }
        }
        return super.onOptionsItemSelected(item)
    }

    override fun onClick(view: CustomView) {
        if (gameManager.isWorking) return

        if (view is PieceView) {
            selectPiece(view)
        } else if (view is TileView
            && view.state == TileView.State.POSSIBLE
            && canMove
            && selectedPos.isValid
        ) {
            val moves = view.storedMoves
            if (moves.isNotEmpty())
                movePiece(moves)
        }
    }

    private fun clearTiles(clearMoved: Boolean = false) {
        tiles.forEach {
            if (clearMoved && it.value.lastMoved)
                it.value.lastMoved = false

            it.value.state = TileView.State.NONE
            it.value.storedMoves.clear()
        }
    }

    private fun setKingInCheck(white: Boolean) {
        tiles.forEach {
            it.value.isInCheck = false
        }
        val drawable = if (white) R.drawable.w_king else R.drawable.b_king

        pieces.forEach {
            if (it.value.pieceImage == drawable) {
                val pos = it.key
                tiles[pos]?.isInCheck = true
            }
        }
    }

    private fun movePiece(moves: List<Move>) {
        fun afterMoveMade() {
            clearTiles(true)
            selectedPos = Pos()

            showLoadingBar(true)
        }

        if (moves.size == 1) {
            gameManager.makeMove(moves.first())
            afterMoveMade()
            return
        }

        // This should only be possible on promotions
        check(moves.all { it.flags.promotion }) { "All moves should be promotions in this case" }
        check(moves.size == 4) { "There should be 4 promotions possible" }

        val dialogBinding = DialogPromotionBinding.inflate(layoutInflater, null, false)
        val resourceOffset = (if (Native.isPlayerWhite()) 0 else 6) - 1

        val dialog = AlertDialog.Builder(this)
            .setTitle(R.string.promotion_choose_piece)
            .setView(dialogBinding.root)
            .create()

        dialogBinding.ivQueen.setImageResource(PieceResourceManager.piecesResources[Piece.QUEEN + resourceOffset])
        dialogBinding.ivQueen.setOnClickListener {
            gameManager.makeMove(moves.first { it.promotedPieceType == Piece.QUEEN })
            afterMoveMade()
            dialog.dismiss()
        }

        dialogBinding.ivRook.setImageResource(PieceResourceManager.piecesResources[Piece.ROOK + resourceOffset])
        dialogBinding.ivRook.setOnClickListener {
            gameManager.makeMove(moves.first { it.promotedPieceType == Piece.ROOK })
            afterMoveMade()
            dialog.dismiss()
        }

        dialogBinding.ivBishop.setImageResource(PieceResourceManager.piecesResources[Piece.BISHOP + resourceOffset])
        dialogBinding.ivBishop.setOnClickListener {
            gameManager.makeMove(moves.first { it.promotedPieceType == Piece.BISHOP })
            afterMoveMade()
            dialog.dismiss()
        }

        dialogBinding.ivKnight.setImageResource(PieceResourceManager.piecesResources[Piece.KNIGHT + resourceOffset])
        dialogBinding.ivKnight.setOnClickListener {
            gameManager.makeMove(moves.first { it.promotedPieceType == Piece.KNIGHT })
            afterMoveMade()
            dialog.dismiss()
        }

        dialogBinding.btnClose.setOnClickListener {
            dialog.dismiss()
        }

        dialog.show()
    }

    private fun selectPiece(view: PieceView) {
        clearTiles()

        selectedPos = view.pos
        tiles[selectedPos]?.state = TileView.State.SELECTED

        val possibleMoves = gameManager.getPossibleMoves(selectedPos)

        possibleMoves.forEach { move ->
            tiles[Pos(move.to)]?.let {
                it.state = TileView.State.POSSIBLE
                it.storedMoves.add(move)
            }
        }
    }

    private fun updateState(gameState: GameState) {
        canMove = true
        val tvDebug = binding.tvDebug

        if (gameManager.basicStatsEnabled) {
            tvDebug.visibility = View.VISIBLE

            val advancedStats =
                if (gameManager.advancedStatsEnabled) "\n" + Native.getAdvancedStats() else ""

            tvDebug.text = getString(
                R.string.basic_stats,
                Native.getSearchTime() / 1000, // Display in Seconds
                Native.getCurrentBoardValue(),
                advancedStats
            )
        } else {
            tvDebug.visibility = View.GONE
        }

        val messageRes = when (gameState) {
            GameState.WINNER_WHITE -> R.string.victory_white
            GameState.WINNER_BLACK -> R.string.victory_black
            GameState.DRAW -> R.string.draw
            else -> 0
        }

        if (messageRes != 0) {
            canMove = false
            showLoadingBar(false)

            AlertDialog.Builder(this)
                .setTitle(messageRes)
                .setPositiveButton(android.R.string.ok, null)
                .show()
        } else {
            showLoadingBar()
        }

        if (gameState == GameState.WHITE_IN_CHESS || gameState == GameState.WINNER_BLACK) {
            setKingInCheck(true)
        } else if (gameState == GameState.BLACK_IN_CHESS || gameState == GameState.WINNER_WHITE) {
            setKingInCheck(false)
        } else {
            tiles.forEach {
                it.value.isInCheck = false
            }
        }
    }

    private fun showNewGameDialog() {
        val dialogBinding = DialogNewGameBinding.inflate(layoutInflater)

        dialogBinding.spDifficulty.setSelection(preferences.difficultyLevel)
        val randomSideIcon = dialogBinding.btnSideRandom.icon
        if (randomSideIcon is LayerDrawable) {
            for (i in 0 until randomSideIcon.numberOfLayers) {
                val layer = randomSideIcon.getDrawable(i)
                if (layer is ClipDrawable)
                    layer.level = 5000
            }
        }

        AlertDialog.Builder(this)
            .setTitle(R.string.new_game)
            .setView(dialogBinding.root)
            .setPositiveButton(R.string.action_start) { _, _ ->
                val playerWhite = when (dialogBinding.tgSide.checkedButtonId) {
                    dialogBinding.btnSideWhite.id -> true
                    dialogBinding.btnSideBlack.id -> false
                    else -> Random.nextBoolean()
                }

                val level = dialogBinding.spDifficulty.selectedItemPosition
                preferences.difficultyLevel = level

                val newSettings = getDifficulty(level, preferences.engineSettings)
                gameManager.updateSettings(newSettings)
                preferences.engineSettings = newSettings

                restartGame(playerWhite)
            }
            .setNegativeButton(android.R.string.cancel, null)
            .show()
    }

    private fun restartGame(isPlayerWhite: Boolean) {
        if (restarting) return
        restarting = true

        val restart = {
            gameManager.initBoard(isPlayerWhite)
            clearTiles(true)
            updateState(GameState.NONE)
            capturedPieces.reset()
            canMove = true
            restarting = false
            binding.tvDebug.text = null
        }

        if (gameManager.isWorking) {
            Native.stopSearch()
            thread {
                while (gameManager.isWorking)
                    Thread.sleep(20)

                runOnUiThread(restart)
            }
        } else restart()
    }

    override fun redrawBoard(isPlayerWhite: Boolean) {
        tiles.forEach {
            binding.layoutBoard.removeView(it.value)
        }
        tiles.clear()

        val boardAppearance = preferences.boardAppearance

        for (i in 0 until 64) {
            val pos = Pos(i % 8, i / 8)
            val isWhite = (pos.x + pos.y) % 2 == 1

            val xSize = invertIf(!isPlayerWhite, pos.x) * viewSize
            val ySize = invertIf(isPlayerWhite, pos.y) * viewSize

            val tileView = TileView(this, isWhite, pos, pieces, this).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xSize.toFloat()
                y = ySize.toFloat()
                appearance = boardAppearance
                this.isPlayerWhite = isPlayerWhite
            }

            tiles[pos] = tileView
            binding.layoutBoard.addView(tileView)
        }
    }

    override fun redrawPieces(newPieces: List<Piece>, isPlayerWhite: Boolean) {
        pieces.forEach {
            binding.layoutBoard.removeView(it.value)
        }
        pieces.clear()

        newPieces.forEach {
            val isWhite = it.type in 1..6
            val resource = PieceResourceManager.piecesResources[it.type.toInt() - 1]
            val clickable = isWhite == isPlayerWhite

            val pos = Pos(it.pos.toByte())

            val xSize = invertIf(!isPlayerWhite, pos.x) * viewSize
            val ySize = invertIf(isPlayerWhite, pos.y) * viewSize

            val pieceView = PieceView(this, resource, pos, this.takeIf { clickable }).apply {
                layoutParams = FrameLayout.LayoutParams(viewSize, viewSize)
                x = xSize.toFloat()
                y = ySize.toFloat()
            }

            pieces[pos] = pieceView
            binding.layoutBoard.addView(pieceView)
        }
    }

    override fun onMove(gameState: GameState) {
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
                binding.layoutBoard.removeView(destView)

                val type = PieceResourceManager.piecesResources.indexOf(destView.pieceImage) + 1
                val pos = Pos(
                    invertIf(!isPlayerWhite, destView.pos.x),
                    invertIf(isPlayerWhite, destView.pos.y)
                )
                val piece = Piece(pos.toInt(), type.toByte())

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

    private fun showLoadingBar(show: Boolean = gameManager.isWorking) {
        binding.pbLoading.visibility = if (show) View.VISIBLE else View.INVISIBLE
    }

    companion object {
        private const val DEFAULT_DIFFICULTY_LEVEL = 4

        private fun invertIf(invert: Boolean, i: Int) = if (invert) 7 - i else i

        private fun getDifficulty(level: Int, currentSettings: EngineSettings): EngineSettings {
            require(level >= 0)

            return currentSettings.copy(
                searchDepth = if (level == 0 || level == 1) level + 2 else level + 3,
                doQuietSearch = level != 0
            )
        }
    }
}
