package net.theluckycoder.chess.ui.home

import android.content.Intent
import android.content.res.Configuration
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.ExperimentalAnimationApi
import androidx.compose.animation.expandIn
import androidx.compose.animation.graphics.ExperimentalAnimationGraphicsApi
import androidx.compose.animation.graphics.res.animatedVectorResource
import androidx.compose.animation.shrinkOut
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material.DropdownMenu
import androidx.compose.material.DropdownMenuItem
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.material.MaterialTheme
import androidx.compose.material.Scaffold
import androidx.compose.material.Text
import androidx.compose.material.TopAppBar
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.dimensionResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import kotlinx.coroutines.delay
import net.theluckycoder.chess.R
import net.theluckycoder.chess.common.cpp.Native
import net.theluckycoder.chess.common.ui.CapturedPiecesLists
import net.theluckycoder.chess.common.ui.ChessBoard
import net.theluckycoder.chess.common.ui.MovesHistory
import net.theluckycoder.chess.common.viewmodel.HomeViewModel
import net.theluckycoder.chess.ui.preferences.PreferencesActivity
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

    HomeDialogs()

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
    }
}

@Composable
private fun HomeChessBoard(
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

@OptIn(ExperimentalAnimationApi::class, ExperimentalAnimationGraphicsApi::class)
@Preview
@Composable
private fun TopBar(viewModel: HomeViewModel = viewModel()) = TopAppBar(
    modifier = Modifier
        .fillMaxWidth()
        .height(dimensionResource(id = R.dimen.toolbar_height)),
    backgroundColor = MaterialTheme.colors.primary,
    title = {
        Text(
            text = stringResource(id = R.string.app_name),
            fontSize = 20.sp,
            fontWeight = FontWeight.SemiBold,
            modifier = Modifier.padding(end = 16.dp)
        )

        val isEngineThinking by viewModel.isEngineBusy.collectAsState()

        AnimatedVisibility(
            visible = isEngineThinking,
            enter = expandIn(expandFrom = Alignment.CenterStart),
            exit = shrinkOut(shrinkTowards = Alignment.CenterStart),
        ) {
            val icon = animatedVectorResource(R.drawable.ic_animated_hourglass)
            var atEnd by remember { mutableStateOf(false) }

            Icon(
                painter = icon.painterFor(atEnd = atEnd),
                modifier = Modifier.size(18.dp),
                contentDescription = null,
            )

            LaunchedEffect(atEnd) {
                delay(250)
                atEnd = true
            }
        }
    },
    actions = {
        AppBarActions()
    }
)

@Composable
private fun AppBarActions(viewModel: HomeViewModel = viewModel()) {
    var showActionsMenu by remember { mutableStateOf(false) }

    IconButton(onClick = { showActionsMenu = true }) {
        Icon(
            painter = painterResource(id = R.drawable.ic_more_options_vertical),
            contentDescription = stringResource(id = R.string.action_more_options)
        )

        if (showActionsMenu) {
            DropdownMenu(
                expanded = showActionsMenu,
                onDismissRequest = { showActionsMenu = false }
            ) {
                DropdownMenuItem(onClick = {
                    showActionsMenu = false
                    viewModel.showImportDialog.value = true
                }) { Text(text = stringResource(id = R.string.fen_position_load)) }

                val isEngineThinking by viewModel.isEngineBusy.collectAsState()
                val basicDebug by viewModel.dataStore.showBasicDebug().collectAsState(false)

                if (basicDebug) {
                    DropdownMenuItem(onClick = {
                        showActionsMenu = false
                        Native.makeEngineMove()
                    }) { Text(text = stringResource(id = R.string.action_make_engine_move)) }

                    if (isEngineThinking) {
                        DropdownMenuItem(onClick = {
                            showActionsMenu = false
                            Native.stopSearch()
                        }) { Text(text = stringResource(id = R.string.action_stop_search)) }
                    }
                }
            }
        }
    }
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

        IconButton(
            onClick = { viewModel.showShareDialog.value = true }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_share),
                contentDescription = null
            )
        }

        val context = LocalContext.current
        IconButton(
            onClick = { context.startActivity(Intent(context, PreferencesActivity::class.java)) }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_settings),
                contentDescription = stringResource(id = R.string.title_settings)
            )
        }
    }
}
