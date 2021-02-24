package net.theluckycoder.chess

import android.app.Application
import androidx.annotation.Keep
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.asLiveData
import kotlinx.coroutines.flow.MutableStateFlow
import net.theluckycoder.chess.model.EngineSettings
import net.theluckycoder.chess.model.GameState
import net.theluckycoder.chess.model.Move
import net.theluckycoder.chess.model.Piece
import net.theluckycoder.chess.model.Pos
import net.theluckycoder.chess.model.PosPair
import net.theluckycoder.chess.model.Tile
import java.util.concurrent.atomic.AtomicBoolean

class ChessViewModel(application: Application) : AndroidViewModel(application) {

    private var initialized = AtomicBoolean(false)

    private val playerPlayingWhiteFlow = MutableStateFlow(true)

    @OptIn(ExperimentalStdlibApi::class)
    private val tilesFlow = MutableStateFlow(buildList {
        for (i in 0 until 64)
            add(Tile(i, Tile.State.None))
    })

    private val piecesFlow = MutableStateFlow(emptyList<Piece>())

    private val gameStateFlow = MutableStateFlow(GameState.NONE)

    val playerPlayingWhite = playerPlayingWhiteFlow.asLiveData()
    val tiles = tilesFlow.asLiveData()
    val pieces = piecesFlow.asLiveData()
    val gameState = gameStateFlow.asLiveData()

    var basicStatsEnabled = false
    var advancedStatsEnabled = false
        set(value) {
            if (field != value) {
                field = value
                Native.enableStats(value)
            }
        }

    init {
        initBoard()
    }

    fun initBoard(playerWhite: Boolean = true) {
        if (initialized.get()) {
            initBoardNative(playerWhite)
        } else {
            // First time it is called, load the last game
            initialized.set(true)

            initBoardNative(playerWhite)

            playerPlayingWhiteFlow.value =
                if (SaveManager.loadFromFile(getApplication())) Native.isPlayerWhite() else playerWhite
        }

        updatePiecesList()
    }

    private fun updatePiecesList() {
        piecesFlow.value = Native.getPieces().filter { it.type.toInt() != 0 }
    }

    fun updateSettings(engineSettings: EngineSettings) {
        Native.setSettings(
            engineSettings.searchDepth,
            engineSettings.threadCount,
            engineSettings.cacheSize,
            engineSettings.doQuietSearch
        )
    }

    fun makeMove(move: Move) {
        Native.makeMove(move.content)
    }

    fun getPossibleMoves(pos: Pos) {
        val moves = Native.getPossibleMoves(pos.toByte())

        tilesFlow.value = tilesFlow.value
            .map { tile ->
                val move = moves.find { it.to.toInt() == tile.index }
                when {
                    move != null -> tile.copy(state = Tile.State.PossibleMove(move))
                    tile.state is Tile.State.PossibleMove -> tile.copy(state = Tile.State.None)
                    else -> tile
                }
            }
    }

    private fun clearTiles() {
        tilesFlow.value = tilesFlow.value.map { it.copy(state = Tile.State.None) }
    }

    @Suppress("unused")
    @Keep // Called by native code
    private fun callback(gameState: Int, shouldRedrawPieces: Boolean, moves: Array<PosPair>) {
        val app = getApplication<Application>()

        val state = when (gameState) {
            1 -> GameState.WINNER_WHITE
            2 -> GameState.WINNER_BLACK
            3 -> GameState.DRAW
            4 -> GameState.WHITE_IN_CHESS
            5 -> GameState.BLACK_IN_CHESS
            else -> GameState.NONE
        }

        SaveManager.saveToFileAsync(app)
        gameStateFlow.value = state
        updatePiecesList()
    }

    private external fun initBoardNative(playerPlayingWhite: Boolean)
}
