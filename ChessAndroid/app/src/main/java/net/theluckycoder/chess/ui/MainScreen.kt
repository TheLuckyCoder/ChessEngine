package net.theluckycoder.chess.ui

import android.content.Intent
import android.content.res.Configuration
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
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
fun MainScreen() = BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
    val boardSize =
        with(LocalDensity.current) { min(constraints.maxWidth, constraints.maxHeight).toDp() }
    val tileSize = boardSize / 8
    BoardTiles(tileSize)
    BoardPieces(tileSize)

    when (LocalConfiguration.current.orientation) {
        Configuration.ORIENTATION_LANDSCAPE -> LandscapeScreen()
        else -> PortraitScreen()
    }

    val chessViewModel = viewModel<ChessViewModel>()
    if (chessViewModel.showNewGameDialog.value) {
        NewGameDialog(chessViewModel)
    }

    val gameState by chessViewModel.gameState.observeAsState(GameState.NONE)
    when (gameState) {
        GameState.DRAW, GameState.WINNER_WHITE, GameState.WINNER_BLACK -> {
            GameFinishedDialog(gameState)
        }
        else -> Unit
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

    var showDialog by mutableStateOf(true)

    if (!showDialog) return

    AlertDialog(
        onDismissRequest = { showDialog = false },
        title = {
            Text(
                text = stringResource(id = messageRes),
                fontSize = 18.sp,
                fontWeight = FontWeight.Bold
            )
        },
        confirmButton = {
            TextButton(onClick = { showDialog = false }) {
                Text(text = stringResource(id = android.R.string.ok))
            }
        }
    )
}

@Preview
@Composable
private fun NewGameDialog(chessViewModel: ChessViewModel = viewModel()) {
    val initialDifficulty = remember { chessViewModel.preferences.difficultyLevel }
    var sideToggleIndex by mutableStateOf(0)
    var difficultyLevel by mutableStateOf(initialDifficulty.toFloat())

    class Side(val painter: Painter, val backgroundColor: Color)

    val sides = listOf(
        Side(painterResource(R.drawable.w_king), colorResource(R.color.side_white)),
        Side(painterResource(R.drawable.b_king), colorResource(R.color.side_black)),
        Side(painterResource(R.drawable.side_random), colorResource(R.color.side_random))
    )

    AlertDialog(
        onDismissRequest = { chessViewModel.showNewGameDialog.value = false },
        title = {
            Text(
                text = stringResource(id = R.string.new_game),
                fontSize = 18.sp,
                fontWeight = FontWeight.Bold
            )
        },
        text = {
            Column(modifier = Modifier.fillMaxWidth()) {
                Text(
                    text = stringResource(id = R.string.side),
                    color = MaterialTheme.colors.secondary
                )

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(8.dp)
                ) {
                    sides.forEachIndexed { index, side ->
                        val backgroundColor =
                            if (sideToggleIndex == index) MaterialTheme.colors.primary.copy(alpha = 0.5f) else side.backgroundColor

                        IconToggleButton(
                            modifier = Modifier
                                .background(backgroundColor)
                                .weight(1f),
                            checked = sideToggleIndex == index,
                            onCheckedChange = { sideToggleIndex = index }
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

                val playerWhite = when (sideToggleIndex) {
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
private fun BoxScope.PortraitScreen(chessViewModel: ChessViewModel = viewModel()) = Column(
    modifier = Modifier.align(Alignment.BottomCenter),
    verticalArrangement = Arrangement.Bottom,
) {
    val isThinking by chessViewModel.isEngineThinking.observeAsState(false)
    if (isThinking) {
        LinearProgressIndicator(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 4.dp)
        )
    }

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(18.dp),
        horizontalArrangement = Arrangement.Center
    ) {
        IconButton(
            onClick = { Native.undoMoves() },
            enabled = false
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_undo),
                contentDescription = null
            )
        }

        NewGameButton(modifier = Modifier.weight(1f))

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
private fun BoxScope.LandscapeScreen() = Column(
    modifier = Modifier
        .fillMaxHeight()
        .align(Alignment.CenterEnd),
    verticalArrangement = Arrangement.Center
) {
    IconButton(
        onClick = { Native.undoMoves() }
    ) {
        Icon(
            painter = painterResource(id = R.drawable.ic_undo),
            contentDescription = null
        )
    }

    NewGameButton(modifier = Modifier.weight(1f))

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

@Preview
@Composable
private fun NewGameButton(
    modifier: Modifier = Modifier,
    chessViewModel: ChessViewModel = viewModel(),
) {
    IconButton(
        modifier = modifier,
        onClick = { chessViewModel.showNewGameDialog.value = true }
    ) {
        Icon(
            painter = painterResource(id = R.drawable.ic_new_circle),
            contentDescription = null
        )
    }
}

private fun getDifficulty(level: Int, currentSettings: EngineSettings): EngineSettings {
    require(level >= 0)

    return currentSettings.copy(
        searchDepth = if (level == 0 || level == 1) level + 2 else level + 3,
        doQuietSearch = level != 0
    )
}
