package net.theluckycoder.chess.wearos.ui.screen

import androidx.compose.animation.graphics.ExperimentalAnimationGraphicsApi
import androidx.compose.animation.graphics.res.animatedVectorResource
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.wear.compose.material.ExperimentalWearMaterialApi
import androidx.wear.compose.material.Icon
import androidx.wear.compose.material.MaterialTheme
import androidx.wear.compose.material.Scaffold
import androidx.wear.compose.material.TimeText
import androidx.wear.compose.material.Vignette
import androidx.wear.compose.material.VignettePosition
import cafe.adriel.voyager.core.screen.Screen
import cafe.adriel.voyager.navigator.LocalNavigator
import cafe.adriel.voyager.navigator.currentOrThrow
import kotlinx.coroutines.delay
import net.theluckycoder.chess.common.cpp.Native
import net.theluckycoder.chess.common.ui.ChessBoard
import net.theluckycoder.chess.common.viewmodel.HomeViewModel
import net.theluckycoder.chess.wearos.R
import net.theluckycoder.chess.wearos.ui.TextIconButton
import net.theluckycoder.chess.wearos.ui.isScreenRound

object WatchScreen : Screen {

    @OptIn(ExperimentalWearMaterialApi::class)
    @Composable
    override fun Content() {
        val viewModel: HomeViewModel = viewModel()
        val listState = rememberLazyListState()

        val isThinking by viewModel.isEngineBusy.collectAsState()

        Scaffold(
            timeText = {
                TimeText(
                    leadingCurvedContent = if (isThinking) {
                        { AnimatedHourGlass() }
                    } else null,
                    leadingLinearContent = if (isThinking) {
                        { AnimatedHourGlass() }
                    } else null,
                )
            },
            vignette = {
                Vignette(vignettePosition = VignettePosition.Top)
            }
        ) {
            LazyColumn(
                modifier = Modifier
                    .background(MaterialTheme.colors.background)
                    .padding(horizontal = 5.dp),
                state = listState,
            ) {
                item {
                    Spacer(Modifier.height(32.dp))
                }

                item {
                    val navigator = LocalNavigator.currentOrThrow

                    Box(Modifier.fillMaxWidth()) {
                        TextIconButton(
                            modifier = Modifier.align(Alignment.Center),
                            onClick = { navigator.push(NewGameScreen()) },
                            text = stringResource(R.string.new_game)
                        ) {
                            Icon(
                                painter = painterResource(R.drawable.ic_new_circle),
                                contentDescription = null
                            )
                        }
                    }
                }

                item {
                    WatchChessBoard(viewModel)
                }

                item {
                    UndoRedoActions(viewModel)
                }

                /*item {
                    SettingsActions()
                }*/

                item {
                    Spacer(Modifier.padding(16.dp))
                }
            }
        }
    }

    @OptIn(ExperimentalAnimationGraphicsApi::class)
    @Composable
    private fun AnimatedHourGlass() {
        val icon = animatedVectorResource(R.drawable.ic_animated_hourglass)
        var atEnd by remember { mutableStateOf(false) }

        Icon(
            painter = icon.painterFor(atEnd = atEnd),
            modifier = Modifier.size(18.dp),
            tint = MaterialTheme.colors.onSurface,
            contentDescription = null,
        )

        LaunchedEffect(Unit) {
            delay(250)
            atEnd = true
        }
    }
}

@Composable
private fun WatchChessBoard(
    viewModel: HomeViewModel = viewModel()
) {
    val isPlayerWhite by viewModel.playerPlayingWhite.collectAsState()
    val tiles by viewModel.tiles.collectAsState()
    val pieces by viewModel.pieces.collectAsState()
    val gameState by viewModel.gameState.collectAsState()

    ChessBoard(
        modifier = Modifier
            .padding(top = 8.dp, bottom = if (isScreenRound()) 8.dp else 2.dp),
        isPlayerWhite = isPlayerWhite,
        tiles = tiles,
        pieces = pieces,
        gameState = gameState,
        onPieceClick = { viewModel.showPossibleMoves(it.square) },
        onShowPromotion = { TODO() }
    )
}

@Composable
private fun UndoRedoActions(
    viewModel: HomeViewModel = viewModel(),
) = Row(
    modifier = Modifier
        .fillMaxWidth()
        .padding(horizontal = 16.dp, vertical = 8.dp),
    horizontalArrangement = Arrangement.SpaceBetween
) {
    val movesIndex by viewModel.currentMoveIndex.collectAsState()
    val movesHistory by viewModel.movesHistory.collectAsState()

    TextIconButton(
        onClick = { Native.undoMoves() },
        enabled = movesIndex >= 0,
        text = stringResource(R.string.action_undo_move),
    ) {
        Icon(
            painter = painterResource(R.drawable.ic_undo),
            contentDescription = stringResource(R.string.action_undo_move)
        )
    }

    TextIconButton(
        onClick = { Native.redoMoves() },
        enabled = movesIndex != movesHistory.lastIndex,
        text = stringResource(R.string.action_redo_move)
    ) {
        Icon(
            painter = painterResource(id = R.drawable.ic_redo),
            contentDescription = stringResource(id = R.string.action_redo_move)
        )
    }
}

/*@Composable
private fun SettingsActions() = Row(
    modifier = Modifier
        .fillMaxWidth()
        .padding(horizontal = 16.dp, vertical = 8.dp),
    horizontalArrangement = Arrangement.SpaceBetween
) {
    val navigator = LocalNavigator.currentOrThrow

    TextIconButton(
        onClick = { navigator.push(NewGameScreen()) },
        text = stringResource(R.string.new_game)
    ) {
        Icon(
            painter = painterResource(R.drawable.ic_new_circle),
            contentDescription = stringResource(R.string.new_game)
        )
    }

    TextIconButton(
        onClick = { navigator.push(SettingsScreen()) },
        text = stringResource(R.string.title_settings),
    ) {
        Icon(
            painter = painterResource(R.drawable.ic_settings),
            contentDescription = stringResource(R.string.title_settings)
        )
    }
}*/
