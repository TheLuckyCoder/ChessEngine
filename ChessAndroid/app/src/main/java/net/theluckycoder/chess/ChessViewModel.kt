package net.theluckycoder.chess

import android.app.Application
import android.util.SparseArray
import androidx.annotation.Keep
import androidx.collection.SparseArrayCompat
import androidx.collection.set
import androidx.compose.runtime.mutableStateOf
import androidx.core.util.set
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.asLiveData
import kotlinx.coroutines.flow.MutableStateFlow
import net.theluckycoder.chess.model.*
import net.theluckycoder.chess.utils.AppPreferences
import java.util.concurrent.atomic.AtomicBoolean

class ChessViewModel(application: Application) : AndroidViewModel(application) {

    private val initialized = AtomicBoolean(false)

    /*
     * Chess Game Data
     */
    private val playerPlayingWhiteFlow = MutableStateFlow(true)
    private val isEngineThinkingFlow = MutableStateFlow(false)

    private val tilesFlow = MutableStateFlow(emptyList<Tile>())
    private val piecesFlow = MutableStateFlow(emptyList<Piece>())
    private val gameStateFlow = MutableStateFlow(GameState.NONE)

    val playerPlayingWhite = playerPlayingWhiteFlow.asLiveData()
    val isEngineThinking = isEngineThinkingFlow.asLiveData()
    val tiles = tilesFlow.asLiveData()
    val pieces = piecesFlow.asLiveData()
    val gameState = gameStateFlow.asLiveData()

    /*
     * UI
     */
    val showNewGameDialog = mutableStateOf(false)

    /*
     * Preferences
     */
    val preferences = AppPreferences

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
            playerPlayingWhiteFlow.value = playerWhite
        } else {
            // First time it is called, load the last game
            initialized.set(true)

            initBoardNative(playerWhite)

            playerPlayingWhiteFlow.value =
                if (SaveManager.loadFromFile(getApplication())) Native.isPlayerWhite() else playerWhite
        }

        clearTiles()
        updatePiecesList()
    }

    private fun updatePiecesList() {
        val pieces = Native.getPieces()
        val piecesMap = mutableMapOf<Int, Piece>()
        // TODO Temporary solution to fix animations
        for (i in 0 until 64) {
            piecesMap[i] = Piece(i, 0, false)
        }

        pieces.forEach {
            piecesMap[it.id] = it.toPiece()
        }

        piecesFlow.value = piecesMap.map { it.value }
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
        isEngineThinkingFlow.value = true
        Native.makeMove(move.content)

        tilesFlow.value = tilesFlow.value
            .map { tile ->
                when {
                    // Mark the moved from and to squares
                    tile.square == move.from.toInt() || tile.square == move.to.toInt() ->
                        tile.copy(state = Tile.State.Moved)
                    // Clear everything else
                    tile.state != Tile.State.None -> tile.copy(state = Tile.State.None)
                    else -> tile
                }
            }
    }

    fun getPossibleMoves(square: Int) {
        val moves = Native.getPossibleMoves(square.toByte())

        tilesFlow.value = tilesFlow.value
            .map { tile ->
                val move = moves.find { it.to.toInt() == tile.square }
                when {
                    tile.square == square && moves.isNotEmpty() -> // Mark the selected piece's square
                        tile.copy(state = Tile.State.Selected)
                    move != null -> // Mark all Possible Moves
                        tile.copy(state = Tile.State.PossibleMove(move))
                    tile.state is Tile.State.PossibleMove || tile.state is Tile.State.Selected -> {
                        // Clear any invalid Possible Moves
                        tile.copy(state = Tile.State.None)
                    }
                    else -> tile
                }
            }
    }

    private fun clearTiles() {
        val tiles = ArrayList<Tile>(64)
        for (i in 0 until 64)
            tiles.add(Tile(i, Tile.State.None))
        tilesFlow.value = tiles
    }

    @Suppress("unused")
    @Keep // Called by native code
    private fun callback(gameState: Int) {
        val app = getApplication<Application>()

        val state = when (gameState) {
            1 -> GameState.WINNER_WHITE
            2 -> GameState.WINNER_BLACK
            3 -> GameState.DRAW
            4 -> GameState.WHITE_IN_CHESS
            5 -> GameState.BLACK_IN_CHESS
            else -> GameState.NONE
        }

        playerPlayingWhiteFlow.value = Native.isPlayerWhite()
        isEngineThinkingFlow.value = Native.isWorking()
        clearTiles()
        updatePiecesList()
        gameStateFlow.value = state

        SaveManager.saveToFileAsync(app)
    }

    private external fun initBoardNative(playerPlayingWhite: Boolean)
}
