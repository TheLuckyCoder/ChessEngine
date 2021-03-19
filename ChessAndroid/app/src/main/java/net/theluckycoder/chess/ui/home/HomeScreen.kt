package net.theluckycoder.chess.ui.home

import android.content.Intent
import android.content.res.Configuration
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.ExperimentalAnimationApi
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
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
import kotlinx.coroutines.launch
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.ui.preferences.PreferencesActivity
import kotlin.time.ExperimentalTime

@Composable
fun HomeScreen() = when (LocalConfiguration.current.orientation) {
    Configuration.ORIENTATION_LANDSCAPE -> {
        Scaffold(
            modifier = Modifier.fillMaxSize(),
            topBar = { TopBar() },
        ) { padding ->
            Row(
                Modifier
                    .padding(padding)
                    .fillMaxSize()
            ) {
                BottomBar(
                    modifier = Modifier
                        .fillMaxHeight()
                        .weight(1f)
                )
                ChessBoard()

                HomeDialogs()
            }
        }
    }
    else -> {
        Scaffold(
            modifier = Modifier.fillMaxSize(),
            topBar = { TopBar() },
            bottomBar = { BottomBar() }
        ) { padding ->
            ChessBoard(Modifier.padding(padding))

            HomeDialogs()
        }
    }
}

@OptIn(ExperimentalAnimationApi::class)
@Preview
@Composable
private fun TopBar(
    chessViewModel: ChessViewModel = viewModel()
) = Column(modifier = Modifier.fillMaxWidth()) {
    TopAppBar(
        modifier = Modifier.height(dimensionResource(id = R.dimen.toolbar_height)),
        backgroundColor = MaterialTheme.colors.primary,
        title = {
            Text(
                text = stringResource(id = R.string.app_name),
                fontSize = 20.sp,
                fontWeight = FontWeight.SemiBold,
                modifier = Modifier.padding(end = 16.dp)
            )

            val isThinking by chessViewModel.isEngineThinking.collectAsState(false)
            AnimatedVisibility(visible = isThinking) {
                Icon(
                    painter = painterResource(id = R.drawable.ic_engine_working),
                    modifier = Modifier.size(18.dp),
                    contentDescription = null,
                )
            }
        },
        actions = {
            AppBarActions()
        }
    )

    MovesHistory(chessViewModel)
}

@Composable
private fun MovesHistory(chessViewModel: ChessViewModel = viewModel()) {
    val movesHistory by chessViewModel.movesHistory.collectAsState()
    val listState = rememberLazyListState()
    val coroutineScope = rememberCoroutineScope()

    remember(movesHistory) {
        coroutineScope.launch {
            listState.animateScrollToItem(movesHistory.size - 1)
        }
    }

    LazyRow(
        state = listState,
        modifier = Modifier
            .fillMaxWidth()
            .padding(4.dp),
        content = {
            if (movesHistory.isEmpty()) {
                item {
                    Text(text = "", fontSize = 13.sp)
                }
            } else {
                itemsIndexed(movesHistory) { index, item ->
                    val padding =
                        if (index % 2 == 0) Modifier.padding(start = 4.dp) else Modifier.padding(end = 4.dp)

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
                        Text(
                            text = item.toString(),
                            fontSize = 13.sp,
                        )
                    }
                }
            }
        }
    )
}

@Composable
private fun AppBarActions() {
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
                    Native.forceMove()
                }) {
                    Text(text = stringResource(id = R.string.action_force_move))
                }
            }
        }
    }
}

@OptIn(ExperimentalTime::class)
@Composable
private fun BottomBar(
    modifier: Modifier = Modifier,
    chessViewModel: ChessViewModel = viewModel()
) = Column(
    modifier = modifier,
    verticalArrangement = Arrangement.Bottom,
    horizontalAlignment = Alignment.CenterHorizontally,
) {
    val basicDebug by chessViewModel.dataStore.showBasicDebug().collectAsState(false)

    if (basicDebug) {
        val debugStats by chessViewModel.debugStats.collectAsState()

        Text(
            text = stringResource(
                id = R.string.debug_stats,
                debugStats.searchTimeNeeded.toString(),
                debugStats.boardEvaluation,
                debugStats.advancedStats
            ),
            fontSize = 13.sp,
        )
    }

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween
    ) {
        IconButton(onClick = { Native.undoMoves() }) {
            Icon(
                painter = painterResource(id = R.drawable.ic_undo),
                contentDescription = stringResource(id = R.string.action_undo_move)
            )
        }

        IconButton(onClick = { Native.redoMoves() }) {
            Icon(
                painter = painterResource(id = R.drawable.ic_redo),
                contentDescription = stringResource(id = R.string.action_redo_move)
            )
        }

        IconButton(
            onClick = { chessViewModel.showNewGameDialog.value = true }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_new_circle),
                contentDescription = stringResource(id = R.string.new_game)
            )
        }

        IconButton(
            onClick = { chessViewModel.showImportExportDialog.value = true }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_import_export),
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
