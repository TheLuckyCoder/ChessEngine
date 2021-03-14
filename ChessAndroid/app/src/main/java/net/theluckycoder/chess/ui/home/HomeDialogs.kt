package net.theluckycoder.chess.ui.home

import android.widget.Toast
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.model.GameState
import net.theluckycoder.chess.ui.AlertDialogTitle
import net.theluckycoder.chess.ui.ChooseSidesToggle
import kotlin.concurrent.thread
import kotlin.math.roundToInt
import kotlin.random.Random

@Composable
fun HomeDialogs(chessViewModel: ChessViewModel = viewModel()) {

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
