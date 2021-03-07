package net.theluckycoder.chess.ui

import android.content.Intent
import android.content.res.Configuration
import android.widget.Toast
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.ExperimentalAnimationApi
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.dimensionResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.model.GameState
import kotlin.concurrent.thread
import kotlin.math.roundToInt
import kotlin.random.Random
import kotlin.time.ExperimentalTime

@Composable
fun MainScreen() = when (LocalConfiguration.current.orientation) {
    Configuration.ORIENTATION_LANDSCAPE -> {
        Scaffold(
            modifier = Modifier.fillMaxSize(),
            topBar = { Toolbar() },
        ) { padding ->
            Row(
                Modifier
                    .padding(padding)
                    .fillMaxSize()
            ) {
                BottomBar(modifier = Modifier.fillMaxHeight().weight(1f))
                ChessBoard()

                Dialogs()
            }
        }
    }
    else -> {
        Scaffold(
            modifier = Modifier.fillMaxSize(),
            topBar = { Toolbar() },
            bottomBar = { BottomBar() }
        ) { padding ->
            ChessBoard(Modifier.padding(padding))

            Dialogs()
        }
    }
}

@Composable
private fun Dialogs(chessViewModel: ChessViewModel = viewModel()) {

    if (chessViewModel.showNewGameDialog.value)
        NewGameDialog(chessViewModel)

    if (chessViewModel.showImportExportDialog.value)
        ImportExportDialog(chessViewModel)

    val gameState by chessViewModel.gameState.collectAsState()
    when (gameState) {
        GameState.DRAW, GameState.WINNER_WHITE, GameState.WINNER_BLACK -> {
            GameFinishedDialog(gameState)
        }
        else -> Unit
    }
}

@OptIn(ExperimentalAnimationApi::class)
@Preview
@Composable
private fun Toolbar() {
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

            val chessViewModel = viewModel<ChessViewModel>()
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

@Composable
private fun GameFinishedDialog(gameState: GameState) {
    val messageRes = when (gameState) {
        GameState.WINNER_WHITE -> R.string.victory_white
        GameState.WINNER_BLACK -> R.string.victory_black
        GameState.DRAW -> R.string.draw
        else -> return
    }

    var showDialog by remember { mutableStateOf(true) }

    if (!showDialog) return

    AlertDialog(
        onDismissRequest = { showDialog = false },
        title = { AlertDialogTitle(text = stringResource(id = messageRes)) },
        confirmButton = {
            TextButton(onClick = { showDialog = false }) {
                Text(text = stringResource(id = android.R.string.ok))
            }
        }
    )
}

@Composable
private fun ImportExportDialog(chessViewModel: ChessViewModel = viewModel()) {
    val currentFen = remember { Native.getCurrentFen() }

    var newFen by remember { mutableStateOf("") }
    val sidesToggleIndex = remember { mutableStateOf(0) }
    var failedToLoad by remember { mutableStateOf(false) }

    AlertDialog(
        onDismissRequest = { chessViewModel.showImportExportDialog.value = false },
        title = { AlertDialogTitle(text = stringResource(id = R.string.fen_position)) },
        text = {
            Column(Modifier.fillMaxWidth()) {
                Text(
                    text = stringResource(id = R.string.fen_position_current),
                    color = MaterialTheme.colors.secondary
                )
                Text(
                    modifier = Modifier.fillMaxWidth(),
                    text = currentFen,
                    fontSize = 15.sp
                )

                val context = LocalContext.current
                val clipboardManager = LocalClipboardManager.current
                TextButton(
                    modifier = Modifier
                        .align(Alignment.End)
                        .padding(top = 4.dp, bottom = 16.dp),
                    onClick = {
                        clipboardManager.setText(AnnotatedString(currentFen))
                        Toast.makeText(context, R.string.fen_position_copied, Toast.LENGTH_SHORT)
                            .show()
                    }
                ) {
                    Text(text = stringResource(id = R.string.action_copy))
                }

                Spacer(
                    modifier = Modifier
                        .fillMaxWidth()
                        .requiredHeight(1.dp)
                        .background(LocalContentColor.current)
                        .clip(RoundedCornerShape(1.dp))
                )

                Text(
                    modifier = Modifier.padding(top = 16.dp),
                    text = stringResource(id = R.string.fen_position_load),
                    color = MaterialTheme.colors.secondary
                )

                TextField(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(top = 4.dp),
                    value = newFen,
                    onValueChange = { newFen = it },
                    placeholder = { Text(text = "FEN") },
                    isError = failedToLoad
                )

                ChooseSidesToggle(sidesToggleIndex = sidesToggleIndex)
            }
        },
        buttons = {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(8.dp)
            ) {
                TextButton(
                    modifier = Modifier.weight(1f),
                    onClick = { chessViewModel.showImportExportDialog.value = false }
                ) {
                    Text(text = stringResource(id = R.string.action_close))
                }

                val context = LocalContext.current
                Button(
                    modifier = Modifier.weight(1f),
                    onClick = {
                        val playerWhite = when (sidesToggleIndex.value) {
                            0 -> true
                            1 -> false
                            else -> Random.nextBoolean()
                        }

                        if (Native.loadFen(playerWhite, newFen)) {
                            failedToLoad = false
                            chessViewModel.showImportExportDialog.value = false
                            Toast.makeText(
                                context,
                                R.string.fen_position_loaded,
                                Toast.LENGTH_SHORT
                            ).show()
                        } else {
                            Toast.makeText(context, R.string.fen_position_error, Toast.LENGTH_LONG)
                                .show()
                            failedToLoad = true
                        }
                    }
                ) {
                    Text(text = stringResource(id = R.string.action_load))
                }
            }
        }
    )
}

@Preview
@Composable
private fun NewGameDialog(chessViewModel: ChessViewModel = viewModel()) {
    val sidesToggleIndex = remember { mutableStateOf(0) }
    var difficultyLevel by remember { mutableStateOf(1f) }

    AlertDialog(
        onDismissRequest = { chessViewModel.showNewGameDialog.value = false },
        title = { AlertDialogTitle(text = stringResource(id = R.string.new_game)) },
        text = {
            Column(modifier = Modifier.fillMaxWidth()) {
                Text(
                    text = stringResource(id = R.string.side),
                    color = MaterialTheme.colors.secondary
                )

                ChooseSidesToggle(sidesToggleIndex = sidesToggleIndex)

                Text(
                    modifier = Modifier.padding(top = 16.dp),
                    text = stringResource(R.string.difficulty_level, difficultyLevel.roundToInt()),
                    color = MaterialTheme.colors.secondary
                )

                Slider(
                    modifier = Modifier.padding(8.dp),
                    value = difficultyLevel,
                    onValueChange = { difficultyLevel = it },
                    valueRange = 1f..8f,
                    steps = 6,
                )
            }
        },
        confirmButton = {
            TextButton(onClick = {
                val playerWhite = when (sidesToggleIndex.value) {
                    0 -> true
                    1 -> false
                    else -> Random.nextBoolean()
                }

                val level = difficultyLevel.roundToInt()
                chessViewModel.updateDifficulty(level)

                if (Native.isWorking()) {
                    Native.stopSearch()
                    thread {
                        while (Native.isWorking())
                            Thread.sleep(20)

                        chessViewModel.initBoard(playerWhite)
                    }
                } else chessViewModel.initBoard(playerWhite)

                chessViewModel.showNewGameDialog.value = false
            }) {
                Text(text = stringResource(id = R.string.action_start))
            }
        },
        dismissButton = {
            TextButton(onClick = {
                chessViewModel.showNewGameDialog.value = false
            }) {
                Text(text = stringResource(id = android.R.string.cancel))
            }
        }
    )
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
        IconButton(
            onClick = { Native.undoMoves() },
            enabled = false
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_undo),
                contentDescription = stringResource(id = R.string.action_undo_move)
            )
        }

        IconButton(
            onClick = { Native.redoMoves() },
            enabled = false
        ) {
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

@Composable
private fun ChooseSidesToggle(
    modifier: Modifier = Modifier,
    sidesToggleIndex: MutableState<Int>,
) {
    class Side(val painterRes: Int, val backgroundColorRes: Int, val contentDescriptionRes: Int)

    val sides = listOf(
        Side(R.drawable.w_king, R.color.side_white, R.string.side_white),
        Side(R.drawable.b_king, R.color.side_black, R.string.side_black),
        Side(R.drawable.side_random, R.color.side_random, R.string.side_random),
    )

    Row(
        modifier = modifier
            .fillMaxWidth()
            .padding(8.dp)
            .clip(RoundedCornerShape(8.dp))
    ) {
        sides.forEachIndexed { index, side ->
            val backgroundColor = if (sidesToggleIndex.value == index)
                MaterialTheme.colors.primary.copy(alpha = 0.5f)
            else
                colorResource(id = side.backgroundColorRes)

            IconToggleButton(
                modifier = Modifier
                    .background(backgroundColor)
                    .weight(1f)
                    .padding(4.dp),
                checked = sidesToggleIndex.value == index,
                onCheckedChange = { sidesToggleIndex.value = index }
            ) {
                Icon(
                    modifier = Modifier.size(54.dp),
                    painter = painterResource(id = side.painterRes),
                    tint = Color.Unspecified,
                    contentDescription = stringResource(id = side.contentDescriptionRes),
                )
            }
        }
    }
}
