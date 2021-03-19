package net.theluckycoder.chess

import android.app.Application
import androidx.annotation.Keep
import androidx.compose.runtime.mutableStateOf
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ensureActive
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.flow.distinctUntilChanged
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import net.theluckycoder.chess.model.*
import net.theluckycoder.chess.utils.SettingsDataStore
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.time.ExperimentalTime

class ChessViewModel(application: Application) : AndroidViewModel(application) {

    private val initialized = AtomicBoolean(false)
    val dataStore = SettingsDataStore(application)

    /*
     * Chess Game Data
     */
    private val playerPlayingWhiteFlow = MutableStateFlow(true)
    private val isEngineThinkingFlow = MutableStateFlow(false)
    private val tilesFlow = MutableStateFlow(emptyList<Tile>())
    private val piecesFlow = MutableStateFlow(emptyList<IndexedPiece>())
    private val gameStateFlow = MutableStateFlow(GameState.NONE)
    private val movesHistoryFlow = MutableStateFlow(emptyList<Move>())
    private val currentMoveIndexFlow = MutableStateFlow(0)
    private val debugStatsFlow = MutableStateFlow(DebugStats())

    val playerPlayingWhite: StateFlow<Boolean> = playerPlayingWhiteFlow
    val isEngineThinking: StateFlow<Boolean> = isEngineThinkingFlow
    val tiles: StateFlow<List<Tile>> = tilesFlow
    val pieces: StateFlow<List<IndexedPiece>> = piecesFlow
    val gameState: StateFlow<GameState> = gameStateFlow
    val movesHistory: StateFlow<List<Move>> = movesHistoryFlow
    val currentMoveIndex: StateFlow<Int> = currentMoveIndexFlow
    val debugStats: StateFlow<DebugStats> = debugStatsFlow

    /*
     * UI
     */
    val showNewGameDialog = mutableStateOf(false)
    val showImportExportDialog = mutableStateOf(false)

    init {
        initBoard()

        viewModelScope.launch(Dispatchers.IO) {
            dataStore.showAdvancedDebug().distinctUntilChanged().collectLatest {
                ensureActive()
                withContext(Dispatchers.Main) { Native.enableStats(it) }
            }
        }

        viewModelScope.launch(Dispatchers.IO) {
            dataStore.getEngineSettings().distinctUntilChanged().collectLatest {
                ensureActive()
                withContext(Dispatchers.Main) { updateEngineSettings(it) }
            }
        }

        viewModelScope.launch {
            movesHistoryFlow.collectLatest {
                ensureActive()
                val state = gameState.value

                if (it.isNotEmpty() && (state != GameState.WINNER_BLACK || state != GameState.WINNER_WHITE))
                    SaveManager.saveToFileAsync(getApplication(), playerPlayingWhite.value, it)
            }
        }
    }

    fun initBoard(playerWhite: Boolean = true) {
        if (initialized.get()) {
            initBoardNative(playerWhite)
        } else {
            // First time it is called, load the last game
            initialized.set(true)

            initBoardNative(playerWhite)
            SaveManager.loadFromFile(getApplication())
        }

        clearTiles()
    }

    private fun updatePiecesList() {
        piecesFlow.value = Native.getPieces().toList()
    }

    fun updateDifficulty(level: Int) = viewModelScope.launch(Dispatchers.IO) {
        dataStore.setDifficultyLevel(level)
    }

    @OptIn(ExperimentalTime::class)
    private fun updateEngineSettings(engineSettings: EngineSettings) {
        engineSettings.also {
            Native.setSearchOptions(
                it.searchDepth,
                it.quietSearch,
                it.threadCount,
                it.hashSize,
                it.searchTime.toLongMilliseconds(),
            )
        }
    }

    fun makeMove(move: Move) {
        Native.makeMove(move.content)
        isEngineThinkingFlow.value = Native.isWorking()

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
    private fun boardChangedCallback(gameState: Int) {
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
        movesHistoryFlow.value = Native.getMovesHistory().toList()
        currentMoveIndexFlow.value = Native.getCurrentMoveIndex()
        debugStatsFlow.value = DebugStats.get()
    }

    private external fun initBoardNative(playerPlayingWhite: Boolean)
}
