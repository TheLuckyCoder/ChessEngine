package net.theluckycoder.chess.ui

import android.content.Intent
import android.widget.Toast
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
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
import net.theluckycoder.chess.*
import net.theluckycoder.chess.R
import net.theluckycoder.chess.model.EngineSettings
import net.theluckycoder.chess.model.GameState
import kotlin.concurrent.thread
import kotlin.math.min
import kotlin.math.roundToInt
import kotlin.random.Random

@Composable
fun MainScreen() = Scaffold(
    modifier = Modifier.fillMaxSize(),
    topBar = { AppBar() },
    bottomBar = { BottomBar() }
) {
    BoxWithConstraints(
        modifier = Modifier
            .fillMaxSize()
            .padding(it)
    ) {
        val boardSize =
            with(LocalDensity.current) { min(constraints.maxWidth, constraints.maxHeight).toDp() }
        val tileSize = boardSize / 8f
        BoardTiles(tileSize)
        BoardPieces(tileSize)
    }

    val chessViewModel = viewModel<ChessViewModel>()
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

@Preview
@Composable
private fun AppBar() {
    TopAppBar(
        modifier = Modifier.height(dimensionResource(id = R.dimen.toolbar_height)),
        backgroundColor = MaterialTheme.colors.primary,
        title = {
            Text(
                text = stringResource(id = R.string.app_name),
                fontSize = 20.sp,
                fontWeight = FontWeight.SemiBold,
            )
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
fun ImportExportDialog(chessViewModel: ChessViewModel = viewModel()) {
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
    val initialDifficulty = remember { chessViewModel.preferences.difficultyLevel }
    val sidesToggleIndex = remember { mutableStateOf(0) }
    var difficultyLevel by remember { mutableStateOf(initialDifficulty.toFloat()) }

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
                val preferences = chessViewModel.preferences

                val playerWhite = when (sidesToggleIndex.value) {
                    0 -> true
                    1 -> false
                    else -> Random.nextBoolean()
                }

                val level = difficultyLevel.roundToInt()
                preferences.difficultyLevel = level

                val newSettings = getDifficulty(level, preferences.engineSettings)
                chessViewModel.updateSettings(newSettings)
                preferences.engineSettings = newSettings

                if (chessViewModel.isEngineThinking.value == true) {
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

@Composable
private fun BottomBar(chessViewModel: ChessViewModel = viewModel()) = Column(
    verticalArrangement = Arrangement.Bottom,
) {
    /*val isThinking by chessViewModel.isEngineThinking.observeAsState(false)
    if (isThinking) {
        LinearProgressIndicator(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp)
        )
    }*/

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
                contentDescription = stringResource(id = R.string.action_undo)
            )
        }

        IconButton(
            onClick = { Native.undoMoves() },
            enabled = false
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_redo),
                contentDescription = null
            )
        }

        IconButton(
            onClick = { chessViewModel.showNewGameDialog.value = true }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_new_circle),
                contentDescription = null
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
            onClick = { context.startActivity(Intent(context, SettingsActivity::class.java)) }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_settings),
                contentDescription = null
            )
        }
    }
}

@Composable
private fun ChooseSidesToggle(
    modifier: Modifier = Modifier,
    sidesToggleIndex: MutableState<Int>,
) {
    class Side(val painter: Painter, val backgroundColor: Color)

    val sides = listOf(
        Side(painterResource(R.drawable.w_king), colorResource(R.color.side_white)),
        Side(painterResource(R.drawable.b_king), colorResource(R.color.side_black)),
        Side(painterResource(R.drawable.side_random), colorResource(R.color.side_random))
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
                side.backgroundColor

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
                    painter = side.painter,
                    tint = Color.Unspecified,
                    contentDescription = null
                )
            }
        }
    }
}

private fun getDifficulty(level: Int, currentSettings: EngineSettings): EngineSettings {
    require(level >= 0)

    return currentSettings.copy(
        searchDepth = if (level == 0 || level == 1) level + 2 else level + 3,
        doQuietSearch = level != 0
    )
}
