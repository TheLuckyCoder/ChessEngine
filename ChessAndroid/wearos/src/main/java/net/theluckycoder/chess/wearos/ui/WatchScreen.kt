package net.theluckycoder.chess.wearos.ui

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.material.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.common.cpp.Native
import net.theluckycoder.chess.common.ui.ChessBoard
import net.theluckycoder.chess.common.viewmodel.HomeViewModel
import net.theluckycoder.chess.wearos.R
import kotlin.time.ExperimentalTime

@Composable
fun HomeScreen(
    viewModel: HomeViewModel = viewModel()
) {
    val showMovesHistory by viewModel.dataStore.showMoveHistory().collectAsState(false)
    val movesHistory by viewModel.movesHistory.collectAsState()
    val currentMoveIndex by viewModel.currentMoveIndex.collectAsState()
    val showCaptures by viewModel.dataStore.showCapturedPieces().collectAsState(false)
    val centerBoard by viewModel.dataStore.centerBoard().collectAsState(false)

    HomeChessBoard(viewModel)
    /*
    when (LocalConfiguration.current.orientation) {
        Configuration.ORIENTATION_LANDSCAPE -> {
            Scaffold(
                modifier = Modifier.fillMaxSize(),
            ) { padding ->
                Row(
                    Modifier
                        .padding(padding)
                        .fillMaxSize()
                ) {
                    HomeChessBoard(viewModel)

                    Column(
                        modifier = Modifier
                            .fillMaxHeight()
                            .weight(1f)
                    ) {
                        MovesHistory(showMovesHistory, movesHistory, currentMoveIndex)

                        CapturedPiecesLists(
                            modifier = Modifier.fillMaxHeight(),
                            show = showCaptures
                        ) {
                            ActionsBar(
                                Modifier
                                    .weight(1f)
                                    .padding(8.dp)
                            )
                        }
                    }
                }
            }
        }
        else -> {
            Scaffold(
                modifier = Modifier.fillMaxSize(),
                topBar = {
                    Column(modifier = Modifier.fillMaxWidth()) {
                        TopBar()
                        MovesHistory(showMovesHistory, movesHistory, currentMoveIndex)
                    }
                },
                bottomBar = { ActionsBar() }
            ) { padding ->
                Box(
                    Modifier
                        .fillMaxSize()
                        .padding(padding)
                ) {
                    val alignment = if (centerBoard) Alignment.Center else Alignment.TopCenter

                    CapturedPiecesLists(
                        modifier = Modifier.align(alignment),
                        showCaptures
                    ) {
                        HomeChessBoard(viewModel)
                    }
                }
            }
        }
    }*/
}

@Composable
fun HomeChessBoard(
    viewModel: HomeViewModel = viewModel()
) {
    val isPlayerWhite by viewModel.playerPlayingWhite.collectAsState()
    val tiles by viewModel.tiles.collectAsState()
    val pieces by viewModel.pieces.collectAsState()
    val gameState by viewModel.gameState.collectAsState()

    ChessBoard(
        isPlayerWhite = isPlayerWhite,
        tiles = tiles,
        pieces = pieces,
        gameState = gameState,
        onPieceClick = { viewModel.showPossibleMoves(it.square) }
    )
}

@OptIn(ExperimentalTime::class)
@Composable
private fun ActionsBar(
    modifier: Modifier = Modifier,
    viewModel: HomeViewModel = viewModel()
) = Column(
    modifier = modifier,
    verticalArrangement = Arrangement.Bottom,
    horizontalAlignment = Alignment.CenterHorizontally,
) {
    val basicDebug by viewModel.dataStore.showBasicDebug().collectAsState(false)

    if (basicDebug) {
        val debugStats by viewModel.debugStats.collectAsState()

        Text(
            text = stringResource(
                id = R.string.debug_stats,
                debugStats.searchTimeNeeded.toString(),
                debugStats.boardEvaluation,
                debugStats.advancedStats
            ),
            fontSize = 13.5.sp,
        )
    }

    val movesHistory by viewModel.movesHistory.collectAsState()
    val movesIndex by viewModel.currentMoveIndex.collectAsState()

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        IconButton(
            onClick = { Native.undoMoves() },
            enabled = movesIndex >= 0,
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_undo),
                contentDescription = stringResource(id = R.string.action_undo_move)
            )
        }

        IconButton(
            onClick = { Native.redoMoves() },
            enabled = movesIndex != movesHistory.lastIndex,
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_redo),
                contentDescription = stringResource(id = R.string.action_redo_move)
            )
        }

        IconButton(
            onClick = { viewModel.showNewGameDialog.value = true }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_new_circle),
                contentDescription = stringResource(id = R.string.new_game)
            )
        }
    }
}
