package net.theluckycoder.chess.viewmodel

import android.app.Application
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
import net.theluckycoder.chess.BoardChangeListener
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.model.*
import net.theluckycoder.chess.utils.SaveManager
import net.theluckycoder.chess.utils.SettingsDataStore
import java.util.concurrent.atomic.AtomicBoolean

class HomeViewModel(application: Application) : AndroidViewModel(application), BoardChangeListener {

    private val initialized = AtomicBoolean(false)
    val dataStore = SettingsDataStore.get(application)

    /*
     * Chess Game Data
     */
    private val isEngineThinkingFlow = MutableStateFlow(false)
    private val playerPlayingWhiteFlow = MutableStateFlow(true)
    private val tilesFlow = MutableStateFlow(getEmptyTiles())
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
    val showShareDialog = mutableStateOf(false)
    val showImportDialog = mutableStateOf(false)

    init {
        resetBoard()

        viewModelScope.launch(Dispatchers.IO) {
            launch {
                dataStore.showAdvancedDebug().distinctUntilChanged().collectLatest {
                    ensureActive()
                    DebugStats.enable(it)
                }
            }

            launch {
                dataStore.getEngineSettings().distinctUntilChanged().collectLatest {
                    ensureActive()
                    withContext(Dispatchers.Main) { Native.setSearchOptions(it) }
                }
            }

            launch {
                movesHistoryFlow.collectLatest {
                    ensureActive()
                    val state = gameState.value
                    if (it.isNotEmpty() && (state != GameState.WINNER_BLACK || state != GameState.WINNER_WHITE)) {
                        SaveManager.saveToFileAsync(
                            getApplication(),
                            Native.getStartFen(),
                            playerPlayingWhite.value,
                            it
                        )
                    }
                }
            }
        }
    }

    fun resetBoard(playerWhite: Boolean = true) {
        if (initialized.get()) {
            if (isEngineThinking.value)
                Native.stopSearch()
            Native.initBoard(this, playerWhite)
        } else {
            // First time it is called, load the last game
            initialized.set(true)

            Native.initBoard(this, true)
            SaveManager.loadFromFile(getApplication())
        }
    }

    private fun updatePiecesList() {
        piecesFlow.value = Native.getPieces().toList()
    }

    fun updateDifficulty(level: Int) = viewModelScope.launch(Dispatchers.IO) {
        dataStore.setDifficultyLevel(level)
    }

    fun getPossibleMoves(square: Int) {
        val moves = Native.getPossibleMoves(square.toByte()).toList()

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

    override fun boardChanged(gameStateInt: Int) {
        val gameState = GameState.getState(gameStateInt)

        playerPlayingWhiteFlow.value = Native.isPlayerWhite()

        gameStateFlow.value = gameState
        val movesHistoryList = Native.getMovesHistory().toList()
        movesHistoryFlow.value = movesHistoryList
        val moveIndex = Native.getCurrentMoveIndex()
        currentMoveIndexFlow.value = moveIndex
        debugStatsFlow.value = DebugStats.get()

        val currentIndex = currentMoveIndexFlow.value
        val currentMove = movesHistoryList.getOrNull(currentIndex)

        tilesFlow.value = if (currentMove != null) {
            getEmptyTiles().map {
                if (it.square.toByte() == currentMove.from || it.square.toByte() == currentMove.to)
                    it.copy(state = Tile.State.Moved)
                else it
            }
        } else
            getEmptyTiles()

        updatePiecesList()

        if (!Native.isPlayersTurn())
            Native.makeEngineMove()

        isEngineThinkingFlow.value = Native.isEngineWorking()
    }

    private companion object {
        private val emptyTiles = Array(64) { Tile(it, Tile.State.None) }

        fun getEmptyTiles() = emptyTiles.toList()
    }
}
