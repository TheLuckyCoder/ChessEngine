package net.theluckycoder.chess.ui.home

import android.content.Intent
import android.content.res.Configuration
import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.ExperimentalComposeUiApi
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.animatedVectorResource
import androidx.compose.ui.res.dimensionResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.model.Move
import net.theluckycoder.chess.model.Piece
import net.theluckycoder.chess.ui.ChessBoard
import net.theluckycoder.chess.ui.preferences.PreferencesActivity
import net.theluckycoder.chess.utils.CapturedPieces
import net.theluckycoder.chess.viewmodel.HomeViewModel
import kotlin.time.ExperimentalTime

@Composable
fun HomeScreen(
    viewModel: HomeViewModel = viewModel()
) {
    val showMovesHistory by viewModel.dataStore.showMoveHistory().collectAsState(false)
    val movesHistory by viewModel.movesHistory.collectAsState()
    val currentMoveIndex by viewModel.currentMoveIndex.collectAsState()
    val showCaptures by viewModel.dataStore.showCaptures().collectAsState(false)

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

                    Column(modifier = Modifier.fillMaxHeight().weight(1f)) {
                        MovesHistory(showMovesHistory, movesHistory, currentMoveIndex)

                        CapturedPiecesLists(
                            modifier = Modifier.fillMaxHeight(),
                            show = showCaptures
                        ) {
                            ActionsBar(Modifier.weight(1f))
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
                Box(Modifier.fillMaxSize().padding(padding)) {
                    CapturedPiecesLists(
                        modifier = Modifier.align(Alignment.TopCenter),
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
        gameState = gameState
    )
}

@Composable
private fun CapturedPiecesLists(
    modifier: Modifier = Modifier,
    show: Boolean,
    content: @Composable ColumnScope.() -> Unit
) = Column(modifier) {
    val viewModel = viewModel<HomeViewModel>()

    val isPlayerWhite by viewModel.playerPlayingWhite.collectAsState()
    val pieces by viewModel.pieces.collectAsState()

    val capturedPieces = remember(pieces.size) { CapturedPieces.from(pieces.map { it.toPiece() }) }

    // Top
    if (show) {
        CapturedPieceList(
            if (isPlayerWhite) capturedPieces.capturedByWhite else capturedPieces.capturedByBlack,
            if (isPlayerWhite) capturedPieces.blackScore else capturedPieces.whiteScore,
        )
    }

    content()

    // Bottom
    if (show) {
        CapturedPieceList(
            if (isPlayerWhite) capturedPieces.capturedByBlack else capturedPieces.capturedByWhite,
            if (isPlayerWhite) capturedPieces.whiteScore else capturedPieces.blackScore
        )
    }
}

@Composable
private fun CapturedPieceList(pieces: List<Byte>, score: Int) {
    LazyRow(modifier = Modifier.fillMaxWidth().padding(4.dp).height(24.dp)) {
        items(pieces) { piece ->
            val id = when (piece) {
                Piece.PAWN -> R.drawable.ic_pawn
                Piece.KNIGHT -> R.drawable.ic_knight
                Piece.BISHOP -> R.drawable.ic_bishop
                Piece.ROOK -> R.drawable.ic_rook
                Piece.QUEEN -> R.drawable.ic_queen
                Piece.KING -> R.drawable.ic_king
                else -> throw IllegalStateException("Unknown Piece")
            }

            Icon(
                painter = painterResource(id = id),
                contentDescription = null
            )
        }

        if (score != 0) {
            item { Text(text = "+$score") }
        }
    }
}

@OptIn(ExperimentalAnimationApi::class, ExperimentalComposeUiApi::class)
@Preview
@Composable
private fun TopBar(viewModel: HomeViewModel = viewModel()) = TopAppBar(
    modifier = Modifier.fillMaxWidth().height(dimensionResource(id = R.dimen.toolbar_height)),
    backgroundColor = MaterialTheme.colors.primary,
    title = {
        Text(
            text = stringResource(id = R.string.app_name),
            fontSize = 20.sp,
            fontWeight = FontWeight.SemiBold,
            modifier = Modifier.padding(end = 16.dp)
        )

        val isEngineThinking by viewModel.isEngineThinking.collectAsState()

        AnimatedVisibility(
            visible = isEngineThinking,
            enter = expandIn(Alignment.CenterStart),
            exit = shrinkOut(Alignment.CenterStart),
        ) {
            val scope = rememberCoroutineScope()
            val icon = animatedVectorResource(R.drawable.ic_animated_hourglass)
            var atEnd by remember { mutableStateOf(false) }

            Icon(
                painter = icon.painterFor(atEnd = atEnd),
                modifier = Modifier.size(18.dp),
                contentDescription = null,
            )

            scope.launch {
                delay(250)
                atEnd = true
            }
        }
    },
    actions = {
        AppBarActions()
    }
)

@OptIn(ExperimentalAnimationApi::class)
@Composable
private fun MovesHistory(
    show: Boolean,
    movesHistory: List<Move>,
    currentMoveIndex: Int,
) = AnimatedVisibility(
    visible = show,
    enter = fadeIn() + expandIn(Alignment.TopCenter),
    exit = shrinkOut(Alignment.TopCenter) + fadeOut(),
    initiallyVisible = false,
) {
    val listState = rememberLazyListState()
    val coroutineScope = rememberCoroutineScope()

    remember(movesHistory, currentMoveIndex) {
        coroutineScope.launch {
            if (movesHistory.isNotEmpty() && currentMoveIndex in movesHistory.indices)
                listState.animateScrollToItem(currentMoveIndex)
        }
    }

    LazyRow(
        state = listState,
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF222222))
            .padding(4.dp),
        content = {
            if (movesHistory.isNotEmpty()) {
                itemsIndexed(movesHistory) { index, item ->
                    val padding = if (index % 2 == 0)
                        Modifier.padding(start = 6.dp, end = 2.dp)
                    else
                        Modifier.padding(start = 2.dp, end = 6.dp)

                    Row(
                        modifier = padding
                    ) {
                        if (index % 2 == 0) {
                            Text(
                                text = "${index / 2 + 1}. ",
                                color = Color.Gray,
                                fontSize = 13.sp,
                            )
                        }

                        val modifier = Modifier.padding(1.dp).then(
                            if (currentMoveIndex == index)
                                Modifier
                                    .clip(RoundedCornerShape(2.dp))
                                    .background(Color.Gray)
                            else Modifier
                        )
                        Text(
                            modifier = modifier,
                            text = item.toString(),
                            fontSize = 13.sp,
                        )
                    }
                }
            } else
                item { Text(modifier = Modifier.padding(1.dp), text = "", fontSize = 13.sp) }
        }
    )
}

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

                val isEngineThinking by viewModel.isEngineThinking.collectAsState()
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
