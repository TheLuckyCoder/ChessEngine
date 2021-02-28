package net.theluckycoder.chess

import android.app.Application
import androidx.annotation.Keep
import androidx.compose.runtime.mutableStateOf
import androidx.lifecycle.AndroidViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import net.theluckycoder.chess.model.*
import net.theluckycoder.chess.utils.AppPreferences
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.collections.ArrayList
import kotlin.collections.List
import kotlin.collections.emptyList
import kotlin.collections.filter
import kotlin.collections.forEach
import kotlin.collections.isNotEmpty
import kotlin.collections.map
import kotlin.collections.mutableMapOf
import kotlin.collections.set

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

    val playerPlayingWhite: StateFlow<Boolean> = playerPlayingWhiteFlow
    val isEngineThinking: StateFlow<Boolean> = isEngineThinkingFlow
    val tiles: StateFlow<List<Tile>> = tilesFlow
    val pieces: StateFlow<List<Piece>> = piecesFlow
    val gameState: StateFlow<GameState> = gameStateFlow

    /*
     * UI
     */
    val showNewGameDialog = mutableStateOf(false)
    val showImportExportDialog = mutableStateOf(false)

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
                val possibleMoves = moves.filter { it.to.toInt() == tile.square }
                when {
                    tile.square == square && moves.isNotEmpty() -> // Mark the selected piece's square
                        tile.copy(state = Tile.State.Selected)
                    possibleMoves.isNotEmpty() -> // Mark each possible square
                        tile.copy(state = Tile.State.PossibleMove(possibleMoves))
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
