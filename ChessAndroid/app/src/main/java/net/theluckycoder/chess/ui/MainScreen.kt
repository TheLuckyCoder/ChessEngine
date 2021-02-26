package net.theluckycoder.chess.ui

import android.app.Activity
import android.content.Intent
import android.content.res.Configuration
import android.graphics.drawable.ClipDrawable
import android.graphics.drawable.LayerDrawable
import androidx.appcompat.app.AlertDialog
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.BoxScope
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.material.LinearProgressIndicator
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.ChessActivity
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.SettingsActivity
import net.theluckycoder.chess.databinding.DialogNewGameBinding
import kotlin.concurrent.thread
import kotlin.math.min
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
}

private fun showNewGameDialog(activity: Activity, chessViewModel: ChessViewModel) {
    val dialogBinding =
        DialogNewGameBinding.inflate(activity.layoutInflater)

    val preferences = chessViewModel.preferences
    dialogBinding.spDifficulty.setSelection(preferences.difficultyLevel)
    val randomSideIcon = dialogBinding.btnSideRandom.icon
    if (randomSideIcon is LayerDrawable) {
        for (i in 0 until randomSideIcon.numberOfLayers) {
            val layer = randomSideIcon.getDrawable(i)
            if (layer is ClipDrawable)
                layer.level = 5000
        }
    }

    AlertDialog.Builder(activity)
        .setTitle(R.string.new_game)
        .setView(dialogBinding.root)
        .setPositiveButton(R.string.action_start) { _, _ ->
            val playerWhite = when (dialogBinding.tgSide.checkedButtonId) {
                dialogBinding.btnSideWhite.id -> true
                dialogBinding.btnSideBlack.id -> false
                else -> Random.nextBoolean()
            }

            val level = dialogBinding.spDifficulty.selectedItemPosition
            preferences.difficultyLevel = level

            val newSettings = ChessActivity.getDifficulty(level, preferences.engineSettings)
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
        }
        .setNegativeButton(android.R.string.cancel, null)
        .show()
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
    val activity = LocalContext.current as Activity
    IconButton(
        modifier = modifier,
        onClick = { showNewGameDialog(activity, chessViewModel) }
    ) {
        Icon(
            painter = painterResource(id = R.drawable.ic_new_circle),
            contentDescription = null
        )
    }
}
