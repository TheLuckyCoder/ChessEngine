package net.theluckycoder.chess.ui

import android.app.Activity
import android.content.Intent
import android.content.res.Configuration
import android.graphics.drawable.ClipDrawable
import android.graphics.drawable.LayerDrawable
import android.util.Log
import androidx.appcompat.app.AlertDialog
import androidx.compose.animation.core.animateOffsetAsState
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxScope
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.material.LinearProgressIndicator
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.ChessActivity
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.SettingsActivity
import net.theluckycoder.chess.databinding.DialogNewGameBinding
import net.theluckycoder.chess.model.Piece
import net.theluckycoder.chess.model.Pos
import net.theluckycoder.chess.model.Tile
import kotlin.math.min
import kotlin.random.Random

val PIECES_RESOURCES = intArrayOf(
    R.drawable.w_pawn,
    R.drawable.w_knight,
    R.drawable.w_bishop,
    R.drawable.w_rook,
    R.drawable.w_queen,
    R.drawable.w_king,
    R.drawable.b_pawn,
    R.drawable.b_knight,
    R.drawable.b_bishop,
    R.drawable.b_rook,
    R.drawable.b_queen,
    R.drawable.b_king
)

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

            chessViewModel.initBoard(playerWhite)
        }
        .setNegativeButton(android.R.string.cancel, null)
        .show()
}

@Composable
private fun BoxScope.PortraitScreen(chessViewModel: ChessViewModel = viewModel()) =
    Column(
        modifier = Modifier.align(Alignment.BottomCenter),
        verticalArrangement = Arrangement.Bottom,
    ) {
        val isThinking by chessViewModel.isEngineThinking.observeAsState(false)
        if (isThinking) {
            LinearProgressIndicator(
                modifier = Modifier.fillMaxWidth().padding(horizontal = 16.dp, vertical = 8.dp)
            )
        }

        Row(
            modifier = Modifier.fillMaxWidth().padding(12.dp),
            horizontalArrangement = Arrangement.Center
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
    }

@Composable
private fun BoxScope.LandscapeScreen() {
    Column(
        modifier = Modifier.fillMaxHeight().align(Alignment.CenterEnd),
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

@Composable
private fun getPieceDrawable(piece: Piece): Painter =
    painterResource(PIECES_RESOURCES[piece.type.toInt() - 1])

@Composable
private fun BoardTiles(
    tileSize: Dp,
    chessViewModel: ChessViewModel = viewModel(),
) = Box(Modifier.fillMaxSize()) {
    val isPlayerWhite by chessViewModel.playerPlayingWhite.observeAsState(true)
    val cells by chessViewModel.tiles.observeAsState(emptyList())

    val whiteTileColor = colorResource(id = R.color.tile_white)
    val blackTileColor = colorResource(id = R.color.tile_black)
    val possibleTileColor = colorResource(id = R.color.tile_possible)
    val movedTileColor = colorResource(id = R.color.tile_last_moved)
//    val kingInCheckColor = colorResource(id = R.color.tile_black)

    val possibleTileCircleSize = with(LocalDensity.current) { 8.dp.toPx() }
    val selectedPieceStrokeSize = with(LocalDensity.current) { 4.dp.toPx() }

    cells.forEach { cell ->
        val x = cell.index % 8
        val y = cell.index / 8
        val isWhite = (x + y) % 2 == 1

        val invertedX = invertIf(!isPlayerWhite, x)
        val invertedY = invertIf(isPlayerWhite, y)

        val tileColor = if (isWhite) whiteTileColor else blackTileColor

        val clickableModifier = if (cell.state is Tile.State.PossibleMove) {
            Modifier.clickable {
                chessViewModel.makeMove(cell.state.move)
            }
        } else Modifier

        Canvas(
            modifier = Modifier
                .size(tileSize)
                .offset(tileSize * invertedX, tileSize * invertedY)
                .then(clickableModifier),
        ) {
            drawRect(tileColor)

            when (cell.state) {
                is Tile.State.PossibleMove -> {
                    if (cell.state.move.flags.capture) {
                        drawRect(possibleTileColor)
                    } else
                        drawCircle(possibleTileColor, radius = possibleTileCircleSize)
                }
                Tile.State.Moved -> drawRect(movedTileColor)
                Tile.State.Selected ->
                    drawRect(movedTileColor, style = Stroke(selectedPieceStrokeSize))
                Tile.State.None -> Unit
            }
        }
    }
}

@Composable
private fun BoardPieces(
    tileSize: Dp,
    chessViewModel: ChessViewModel = viewModel()
) = Box(modifier = Modifier.fillMaxSize()) {
    val isPlayerWhite by chessViewModel.playerPlayingWhite.observeAsState(true)
    val pieces by chessViewModel.pieces.observeAsState(emptyList())

    pieces.forEach { piece ->
        val x = piece.index % 8
        val y = piece.index / 8

        val invertedX = invertIf(!isPlayerWhite, x)
        val invertedY = invertIf(isPlayerWhite, y)
        val offset = with(LocalDensity.current) {
            Offset(tileSize.toPx() * invertedX, tileSize.toPx() * invertedY)
        }

        val animatedOffset = animateOffsetAsState(targetValue = offset).value
        val (animatedX, animatedY) = with(LocalDensity.current) { animatedOffset.x.toDp() to animatedOffset.y.toDp() }

        IconButton(
            modifier = Modifier
                .size(tileSize)
                .offset(animatedX, animatedY),
            enabled = isPlayerWhite == piece.isWhite,
            onClick = {
                chessViewModel.getPossibleMoves(Pos(x, y))
            }
        ) {
            Image(painter = getPieceDrawable(piece = piece), contentDescription = null)
        }
    }
}

private fun invertIf(invert: Boolean, i: Int) = if (invert) 7 - i else i
