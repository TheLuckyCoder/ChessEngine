package net.theluckycoder.chess.ui

import android.content.Intent
import android.content.res.Configuration
import android.util.Log
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.ExperimentalFoundationApi
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
import androidx.compose.foundation.layout.size
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.Native
import net.theluckycoder.chess.R
import net.theluckycoder.chess.SettingsActivity
import net.theluckycoder.chess.model.Piece
import net.theluckycoder.chess.model.Pos
import net.theluckycoder.chess.model.Tile
import kotlin.math.min

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
fun MainScreen() = Box(modifier = Modifier.fillMaxSize()) {
    ChessBoard()

    when (LocalConfiguration.current.orientation) {
        Configuration.ORIENTATION_LANDSCAPE -> LandscapeScreen()
        else -> PortraitScreen()
    }
}

@Composable
private fun BoxScope.PortraitScreen() {
    Row(
        modifier = Modifier.fillMaxWidth().align(Alignment.BottomCenter),
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

        IconButton(
            modifier = Modifier.weight(1f),
            onClick = { }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_new_circle),
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

        IconButton(
            modifier = Modifier.weight(1f),
            onClick = { }
        ) {
            Icon(
                painter = painterResource(id = R.drawable.ic_new_circle),
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
private fun getPieceDrawable(piece: Piece): Painter =
    painterResource(PIECES_RESOURCES[piece.type.toInt() - 1])

@OptIn(ExperimentalFoundationApi::class)
@Composable
private fun ChessBoard(
    chessViewModel: ChessViewModel = viewModel()
) = BoxWithConstraints(Modifier.fillMaxSize()) {
    val pxSize = min(constraints.maxWidth, constraints.maxHeight)
    val dpSize = with(LocalDensity.current) { pxSize.toDp() }
//    val tileSizePx = (pxSize / 8).toFloat()
    val tileSizeDp = (dpSize / 8)
//    val tileSize = Size(tileSizePx, tileSizePx)

    val isPlayerWhite by chessViewModel.playerPlayingWhite.observeAsState(true)
    val cells by chessViewModel.tiles.observeAsState(emptyList())
    val pieces by chessViewModel.pieces.observeAsState(emptyList())

    val possibleTileCircleSize = with(LocalDensity.current) { 8.dp.toPx() }
    val possibleTileColor = colorResource(id = R.color.tile_possible)
    val movedTileColor = colorResource(id = R.color.tile_last_moved)
    val whiteTileColor = colorResource(id = R.color.tile_white)
    val blackTileColor = colorResource(id = R.color.tile_black)

    /*Canvas(modifier = Modifier.size(dpSize)) {
          cells.forEach { cell ->
              val x = cell.index % 8
              val y = cell.index / 8
              val isWhite = (x + y) % 2 == 1

              val topLeft = Offset(
                  invertIf(!isPlayerWhite, x) * tileSizePx,
                  invertIf(isPlayerWhite, y) * tileSizePx
              )

              val color = when (cell.state) {
                  Tile.State.POSSIBLE_MOVE, Tile.State.POSSIBLE_CAPTURE -> possibleTileColor
                  Tile.State.MOVED -> movedTileColor
                  else -> if (isWhite) whiteTileColor else blackTileColor
              }

              drawRect(color, topLeft = topLeft, size = tileSize)
          }
      }*/

    cells.forEach { cell ->
        val x = cell.index % 8
        val y = cell.index / 8
        val isWhite = (x + y) % 2 == 1

        val invertedX = invertIf(!isPlayerWhite, x)
        val invertedY = invertIf(isPlayerWhite, y)

        val tileColor = if (isWhite) whiteTileColor else blackTileColor

        val clickableModifier = if (cell.state is Tile.State.PossibleMove) {
            Modifier.clickable {
                Log.v("Hello", "You clicked ${cell.index}")
                chessViewModel.makeMove(cell.state.move)
            }
        } else Modifier

        Canvas(
            modifier = Modifier
                .size(tileSizeDp)
                .offset(tileSizeDp * invertedX, tileSizeDp * invertedY)
                .then(clickableModifier),
        ) {
            drawRect(tileColor)

            when (cell.state) {
                is Tile.State.PossibleMove -> {
                    drawCircle(possibleTileColor, radius = possibleTileCircleSize)
                }
                Tile.State.Moved -> Unit
                Tile.State.Selected -> Unit
                Tile.State.None -> Unit
            }
        }
    }

    pieces.forEach { piece ->
        val x = piece.index % 8
        val y = piece.index / 8

        val invertedX = invertIf(!isPlayerWhite, x)
        val invertedY = invertIf(isPlayerWhite, y)

        IconButton(
            modifier = Modifier
                .size(tileSizeDp)
                .offset(tileSizeDp * invertedX, tileSizeDp * invertedY),
            enabled = isPlayerWhite == piece.isWhite,
            onClick = {
                chessViewModel.getPossibleMoves(Pos(x, y))
            }) {
            Image(painter = getPieceDrawable(piece = piece), contentDescription = null)
        }
    }
}

private fun invertIf(invert: Boolean, i: Int) = if (invert) 7 - i else i
