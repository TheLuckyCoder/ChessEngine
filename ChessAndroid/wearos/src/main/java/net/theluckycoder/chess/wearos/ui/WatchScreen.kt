package net.theluckycoder.chess.wearos.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.wear.compose.material.MaterialTheme
import kotlinx.coroutines.launch
import net.theluckycoder.chess.common.cpp.Native
import net.theluckycoder.chess.common.ui.ChessBoard
import net.theluckycoder.chess.common.viewmodel.HomeViewModel
import net.theluckycoder.chess.wearos.R
import kotlin.time.ExperimentalTime

@OptIn(ExperimentalMaterialApi::class)
@Composable
fun WatchScreen(
    viewModel: HomeViewModel = viewModel()
) {
    val dismissState = rememberDismissState()
    val scope = rememberCoroutineScope()

    SwipeToDismiss(
        modifier = Modifier.fillMaxSize(),
        state = dismissState,
        directions = setOf(DismissDirection.EndToStart),
        background = {
            ActionsScreen(onResume = {
                scope.launch {
                    dismissState.reset()
                }
            })
        }
    ) {
        WatchChessBoard(viewModel)
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
            .background(MaterialTheme.colors.background)
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 6.dp, vertical = 72.dp),
        isPlayerWhite = isPlayerWhite,
        tiles = tiles,
        pieces = pieces,
        gameState = gameState,
        onPieceClick = { viewModel.showPossibleMoves(it.square) }
    )
}

@OptIn(ExperimentalTime::class)
//@Preview(widthDp = 200, heightDp = 200, showBackground = true, uiMode = UI_MODE_TYPE_WATCH)
@Composable
private fun ActionsScreen(
    viewModel: HomeViewModel = viewModel(),
    onResume: () -> Unit
) = Column(
    modifier = Modifier.fillMaxSize(),
    verticalArrangement = Arrangement.SpaceEvenly,
) {
    val movesIndex by viewModel.currentMoveIndex.collectAsState()

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 8.dp),
        horizontalArrangement = Arrangement.SpaceEvenly
    ) {
        TextIconButton(
            onClick = onResume,
            text = stringResource(R.string.action_resume)
        ) {
            Icon(
                painter = painterResource(R.drawable.ic_play),
                tint = Color.White,
                contentDescription = stringResource(R.string.action_resume)
            )
        }

        TextIconButton(
            onClick = { viewModel.showNewGameDialog.value = true },
            text = stringResource(R.string.new_game)
        ) {
            Icon(
                painter = painterResource(R.drawable.ic_new_circle),
                tint = Color.White,
                contentDescription = stringResource(R.string.new_game)
            )
        }
    }

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 8.dp),
        horizontalArrangement = Arrangement.SpaceEvenly
    ) {
        TextIconButton(
            onClick = { Native.undoMoves() },
            enabled = movesIndex >= 0,
            text = stringResource(R.string.action_undo_move),
        ) {
            Icon(
                painter = painterResource(R.drawable.ic_undo),
                tint = Color.White,
                contentDescription = stringResource(R.string.action_undo_move)
            )
        }

        TextIconButton(
            onClick = { },
            text = stringResource(R.string.title_settings),
        ) {
            Icon(
                painter = painterResource(R.drawable.ic_settings),
                tint = Color.White,
                contentDescription = stringResource(R.string.title_settings)
            )
        }
    }
}
