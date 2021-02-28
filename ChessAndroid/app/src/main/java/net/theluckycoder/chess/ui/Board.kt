package net.theluckycoder.chess.ui

import androidx.compose.animation.core.animateOffsetAsState
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.material.AlertDialog
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeJoin
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.R
import net.theluckycoder.chess.model.Move
import net.theluckycoder.chess.model.Piece
import net.theluckycoder.chess.model.Tile

val PIECES_RESOURCES = intArrayOf(
    R.drawable.w_pawn, R.drawable.w_knight, R.drawable.w_bishop,
    R.drawable.w_rook, R.drawable.w_queen, R.drawable.w_king,
    R.drawable.b_pawn, R.drawable.b_knight, R.drawable.b_bishop,
    R.drawable.b_rook, R.drawable.b_queen, R.drawable.b_king
)

@Composable
private fun getPieceDrawable(piece: Piece): Painter {
    val type = if (piece.isWhite) piece.type.toInt() else piece.type.toInt() + 6
    return painterResource(PIECES_RESOURCES[type - 1])
}

@Composable
fun BoardTiles(
    tileSize: Dp,
    chessViewModel: ChessViewModel = viewModel(),
) {
    val showPromotionDialog = remember { mutableStateOf(emptyList<Move>()) }

    val currentDensity = LocalDensity.current
    val isPlayerWhite by chessViewModel.playerPlayingWhite.collectAsState()
    val cells by chessViewModel.tiles.collectAsState()

    val whiteTileColor = colorResource(id = R.color.tile_white)
    val blackTileColor = colorResource(id = R.color.tile_black)
    val possibleTileColor = colorResource(id = R.color.tile_possible)
    val movedTileColor = colorResource(id = R.color.tile_last_moved)

    val possibleTileCircleSize = with(currentDensity) { 8.dp.toPx() }
    val selectedPieceStrokeSize = with(currentDensity) { 4.dp.toPx() }

    val possibleCapturePath = remember(currentDensity) {
        val size = with(currentDensity) { tileSize.toPx() }
        Path().apply {
            drawTriangle(0f, 0f, size / 3f, 0f, 0f, size / 3f)
            drawTriangle(size, 0f, size - size / 3f, 0f, size, size / 3f)
            drawTriangle(0f, size, 0f, size - size / 3, size / 3, size)
            drawTriangle(size, size, size, size - size / 3, size - size / 3, size)
        }
    }

    cells.forEach { cell ->
        val x = cell.square % 8
        val y = cell.square / 8
        val isWhite = (x + y) % 2 == 1

        val invertedX = invertIf(!isPlayerWhite, x)
        val invertedY = invertIf(isPlayerWhite, y)

        val tileColor = if (isWhite) whiteTileColor else blackTileColor

        val clickableModifier = if (cell.state is Tile.State.PossibleMove) {
            Modifier.clickable {
                val moves = cell.state.moves
                when {
                    moves.size > 1 -> showPromotionDialog.value = moves
                    else -> chessViewModel.makeMove(moves.first())
                }
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
                    val moves = cell.state.moves
                    if (moves.first().flags.capture)
                        drawPath(possibleCapturePath, color = possibleTileColor)
                    else
                        drawCircle(possibleTileColor, radius = possibleTileCircleSize)
                }
                Tile.State.Moved -> drawRect(movedTileColor)
                Tile.State.Selected -> {
                    val style = Stroke(selectedPieceStrokeSize, join = StrokeJoin.Round)
                    drawRect(movedTileColor, style = style)
                }
                Tile.State.None -> Unit
            }
        }
    }

    if (showPromotionDialog.value.isNotEmpty())
        PromotionDialog(showPromotionDialog)
}

@Composable
fun BoardPieces(
    tileSize: Dp,
    chessViewModel: ChessViewModel = viewModel()
) {
    val isPlayerWhite by chessViewModel.playerPlayingWhite.collectAsState()
    val pieces by chessViewModel.pieces.collectAsState()
//    val gameState by chessViewModel.gameState.collectAsState()

//    val kingInCheckColor = colorResource(id = R.color.king_in_check)

    for (piece in pieces) {
        if (piece.square >= 64 || piece.type.toInt() == 0) continue
        val x = piece.square % 8
        val y = piece.square / 8

        val invertedX = invertIf(!isPlayerWhite, x)
        val invertedY = invertIf(isPlayerWhite, y)
        val offset = with(LocalDensity.current) {
            Offset(tileSize.toPx() * invertedX, tileSize.toPx() * invertedY)
        }

        val animatedOffset by animateOffsetAsState(targetValue = offset)
        val (animatedX, animatedY) = with(LocalDensity.current) { animatedOffset.x.toDp() to animatedOffset.y.toDp() }

        IconButton(
            modifier = Modifier
                .requiredSize(tileSize)
                .offset(animatedX, animatedY),
            enabled = isPlayerWhite == piece.isWhite,
            onClick = { chessViewModel.getPossibleMoves(piece.square) }
        ) {
            Image(painter = getPieceDrawable(piece = piece), contentDescription = null)
        }
    }
}

private fun invertIf(invert: Boolean, i: Int) = if (invert) 7 - i else i

private fun Path.drawTriangle(
    x1: Float, y1: Float,
    x2: Float, y2: Float,
    x3: Float, y3: Float
) {
    moveTo(x1, y1)
    lineTo(x2, y2)
    lineTo(x3, y3)
    lineTo(x1, y1)
}

@Composable
private fun PromotionDialog(showPromotionDialog: MutableState<List<Move>>) {
    val viewModel = viewModel<ChessViewModel>()
    val piecesDrawables =
        listOf(R.drawable.w_knight, R.drawable.w_bishop, R.drawable.w_rook, R.drawable.w_queen)
    val piecesPainters = piecesDrawables.map { painterResource(id = it) }

    AlertDialog(onDismissRequest = { showPromotionDialog.value = emptyList() },
        title = {
            AlertDialogTitle(text = stringResource(id = R.string.promotion_choose_piece))
        },
        text = {
            Row(modifier = Modifier.fillMaxWidth()) {
                piecesPainters.forEachIndexed { index, painter ->
                    IconButton(onClick = {
                        viewModel.makeMove(showPromotionDialog.value[index])
                    }) {
                        Icon(
                            modifier = Modifier.size(64.dp),
                            tint = Color.Unspecified,
                            painter = painter, contentDescription = null
                        )
                    }
                }
            }
        },
        buttons = {
            IconButton(onClick = { showPromotionDialog.value = emptyList() }) {
                Icon(
                    painter = painterResource(id = R.drawable.ic_close),
                    contentDescription = stringResource(id = android.R.string.cancel)
                )
            }
        }
    )
}
