package net.theluckycoder.chess.ui.home

import androidx.compose.animation.core.animateOffsetAsState
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.Image
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.material.AlertDialog
import androidx.compose.material.Icon
import androidx.compose.material.IconButton
import androidx.compose.material.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.colorResource
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import net.theluckycoder.chess.ChessViewModel
import net.theluckycoder.chess.R
import net.theluckycoder.chess.model.IndexedPiece
import net.theluckycoder.chess.model.Move
import net.theluckycoder.chess.model.Piece
import net.theluckycoder.chess.model.Tile
import net.theluckycoder.chess.ui.AlertDialogTitle
import kotlin.math.min

private val PIECES_RESOURCES = intArrayOf(
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
fun ChessBoard(
    modifier: Modifier = Modifier,
    chessViewModel: ChessViewModel = viewModel()
) = BoxWithConstraints(
    modifier = modifier
) {
    val boardSize =
        with(LocalDensity.current) { min(constraints.maxWidth, constraints.maxHeight).toDp() }
    val tileSize = boardSize / 8f

    val showCoordinates by chessViewModel.dataStore.showCoordinates().collectAsState(false)
    val isPlayerWhite by chessViewModel.playerPlayingWhite.collectAsState()
    val tiles by chessViewModel.tiles.collectAsState()
    val pieces by chessViewModel.pieces.collectAsState()

    BoardTiles(boardSize, tileSize, isPlayerWhite, tiles, chessViewModel)

    BoardPieces(tileSize, isPlayerWhite, pieces, chessViewModel)

    if (showCoordinates)
        BoardCoordinates(tileSize)
}

@Composable
fun BoardTiles(
    boardDp: Dp,
    tileDp: Dp,
    isPlayerWhite: Boolean,
    tiles: List<Tile>,
    chessViewModel: ChessViewModel = viewModel()
) {
    val showPromotionDialog = remember { mutableStateOf(emptyList<Move>()) }

    val currentDensity = LocalDensity.current

    val whiteTileColor = colorResource(id = R.color.tile_white)
    val blackTileColor = colorResource(id = R.color.tile_black)
    val possibleTileColor = colorResource(id = R.color.tile_possible)
    val movedTileColor = colorResource(id = R.color.tile_last_moved)

    val possibleTileCircleSize = with(currentDensity) { 8.dp.toPx() }
    val tilePx = with(currentDensity) { tileDp.toPx() }
    val tileSize = Size(tilePx, tilePx)

    val possibleCapturePath = remember(tilePx) {
        Path().apply {
            drawTriangle(0f, 0f, tilePx / 3f, 0f, 0f, tilePx / 3f)
            drawTriangle(tilePx, 0f, tilePx - tilePx / 3f, 0f, tilePx, tilePx / 3f)
            drawTriangle(0f, tilePx, 0f, tilePx - tilePx / 3, tilePx / 3, tilePx)
            drawTriangle(tilePx, tilePx, tilePx, tilePx - tilePx / 3, tilePx - tilePx / 3, tilePx)
        }
    }

    Canvas(modifier = Modifier.size(boardDp)) {
        tiles.forEach { tile ->
            val isWhite = (tile.square % 8 + tile.square / 8) % 2 == 1
            val tileColor = if (isWhite) whiteTileColor else blackTileColor
            val offset = getBoardOffset(isPlayerWhite, tile.square, tilePx)

            drawRect(tileColor, topLeft = offset, size = tileSize)

            when (tile.state) {
                is Tile.State.PossibleMove -> {
                    val moves = tile.state.moves
                    if (moves.first().flags.capture) {
                        possibleCapturePath.translate(offset)
                        drawPath(path = possibleCapturePath, color = possibleTileColor)
                        possibleCapturePath.translate(-offset)
                    } else {
                        val center = offset + Offset(tilePx / 2f, tilePx / 2f)
                        drawCircle(
                            color = possibleTileColor,
                            radius = possibleTileCircleSize,
                            center = center
                        )
                    }
                }
                Tile.State.Moved, Tile.State.Selected ->
                    drawRect(movedTileColor, topLeft = offset, size = tileSize)
                Tile.State.None -> Unit
            }
        }
    }

    tiles
        .filter { it.state is Tile.State.PossibleMove }
        .forEach { tile ->
            val offset = getBoardOffset(isPlayerWhite, tile.square, tilePx)
            val (x, y) = with(currentDensity) { offset.x.toDp() to offset.y.toDp() }
            val state = tile.state as Tile.State.PossibleMove

            Box(
                modifier = Modifier
                    .size(tileDp)
                    .offset(x, y)
                    .clickable {
                        val moves = state.moves
                        when {
                            moves.size > 1 -> showPromotionDialog.value = moves
                            else -> chessViewModel.makeMove(moves.first())
                        }
                    }
            )
        }

    if (showPromotionDialog.value.isNotEmpty())
        PromotionDialog(showPromotionDialog)
}

@Composable
fun BoardCoordinates(tileDp: Dp) {
    val whiteTileColor = colorResource(id = R.color.tile_white)
    val blackTileColor = colorResource(id = R.color.tile_black)

    val textSize = 14.sp
    for (i in 1..8) {
        val textColor = if (i % 2 == 0) whiteTileColor else blackTileColor
        Text(
            text = i.toString(),
            fontSize = textSize,
            modifier = Modifier.offset(0.dp, tileDp * (i - 1)),
            color = textColor,
        )
    }

    val textSizeDp = with(LocalDensity.current) { 15.5.sp.toDp() }
    for (i in 0..7) {
        val textColor = if (i % 2 == 0) whiteTileColor else blackTileColor
        Text(
            text = ('A' + i).toString(),
            fontSize = textSize,
            modifier = Modifier.offset(tileDp * (i + 1) - textSizeDp, tileDp * 8 - textSizeDp),
            color = textColor,
        )
    }
}

@Composable
fun BoardPieces(
    tileDp: Dp,
    isPlayerWhite: Boolean,
    pieces: List<IndexedPiece>,
    chessViewModel: ChessViewModel = viewModel()
) {
//    val gameState by chessViewModel.gameState.collectAsState()

//    val kingInCheckColor = colorResource(id = R.color.king_in_check)

    val tilePx = with(LocalDensity.current) { tileDp.toPx() }

    for (indexedPiece in pieces) {
        key(indexedPiece.id) {
            val piece = indexedPiece.toPiece()
            if (piece.square < 64 && piece.type.toInt() != 0) {
                val offset = getBoardOffset(isPlayerWhite, piece.square, tilePx)
                val animatedOffset by animateOffsetAsState(targetValue = offset)
                val (animatedX, animatedY) = with(LocalDensity.current) { animatedOffset.x.toDp() to animatedOffset.y.toDp() }

                IconButton(
                    modifier = Modifier
                        .requiredSize(tileDp)
                        .offset(animatedX, animatedY),
                    enabled = isPlayerWhite == piece.isWhite,
                    onClick = { chessViewModel.getPossibleMoves(piece.square) }
                ) {
                    Image(painter = getPieceDrawable(piece = piece), contentDescription = null)
                }
            }
        }
    }
}

@Composable
private fun PromotionDialog(showPromotionDialog: MutableState<List<Move>>) {
    val viewModel = viewModel<ChessViewModel>()

    val promotionResources = intArrayOf(
        R.drawable.w_queen, R.drawable.w_rook,
        R.drawable.w_knight, R.drawable.w_bishop
    )
    val piecesPainters = promotionResources.map { painterResource(id = it) }

    AlertDialog(onDismissRequest = { showPromotionDialog.value = emptyList() },
        title = {
            AlertDialogTitle(text = stringResource(id = R.string.promotion_choose_piece))
        },
        text = {
            Row(modifier = Modifier.fillMaxWidth()) {
                piecesPainters.forEachIndexed { index, painter ->
                    IconButton(
                        modifier = Modifier.weight(1f),
                        onClick = {
                            viewModel.makeMove(showPromotionDialog.value[index])
                            showPromotionDialog.value = emptyList()
                        }
                    ) {
                        Image(
                            modifier = Modifier,
                            painter = painter, contentDescription = null
                        )
                    }
                }
            }
        },
        buttons = {
            IconButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = { showPromotionDialog.value = emptyList() }
            ) {
                Icon(
                    modifier = Modifier.fillMaxWidth(),
                    painter = painterResource(id = R.drawable.ic_close),
                    contentDescription = stringResource(id = android.R.string.cancel)
                )
            }
        }
    )
}

private fun invertIf(invert: Boolean, i: Int) = if (invert) 7 - i else i

private fun getBoardOffset(isPlayerWhite: Boolean, index: Int, tilePx: Float): Offset {
    val x = index % 8
    val y = index / 8

    val invertedX = invertIf(!isPlayerWhite, x)
    val invertedY = invertIf(isPlayerWhite, y)

    return Offset(tilePx * invertedX, tilePx * invertedY)
}

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
