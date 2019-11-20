#include "Player.h"

#include "Board.h"
#include "Piece.h"
#include "../algorithm/MoveGen.h"
#include "../BoardManager.h"

bool Player::isAttacked(const Color colorAttacking, const byte targetSquare, const Board &board)
{
    if (board.getType(colorAttacking, PAWN) & PieceAttacks::getPawnAttacks(oppositeColor(colorAttacking), targetSquare))
		return true;

    if (board.getType(colorAttacking, KNIGHT) & PieceAttacks::getKnightAttacks(targetSquare))
		return true;

	if (board.getType(colorAttacking, KING) & PieceAttacks::getKingAttacks(targetSquare))
		return true;

	if (const U64 bishopsQueens = board.getType(colorAttacking, BISHOP) | board.getType(colorAttacking, QUEEN);
		bishopsQueens & PieceAttacks::getBishopAttacks(targetSquare, board.occupied))
		return true;

    if (const U64 rooksQueens = board.getType(colorAttacking, ROOK) | board.getType(colorAttacking, QUEEN);
		rooksQueens & PieceAttacks::getRookAttacks(targetSquare, board.occupied))
		return true;

	return false;
}

bool Player::onlyKingsLeft(const Board &board)
{
	if (board.npm != 0)
		return false;

	for (const Piece &piece : board.data)
		if (piece && piece.type != PieceType::KING)
			return false;

	return true;
}

bool Player::hasNoValidMoves(const Color color, const Board &board)
{
	const auto pieces = getAllOwnedPieces(color, board);

	for (const auto &pair : pieces)
	{
		const byte startSq = pair.first;
		const Piece &selectedPiece = pair.second;
		const auto possibleMoves = selectedPiece.getPossibleMoves(startSq, board);

		for (const Pos &destPos : possibleMoves)
		{
			if (board[destPos].type == PieceType::KING)
				continue;

			Board tempBoard = board;
			tempBoard.doMove(startSq, destPos.toSquare(), false);

			if (isInCheck(color, tempBoard))
				continue;

			return false;
		}
	}

	return true;
}

bool Player::isInCheck(const Color color, const Board &board)
{
	const U64 king = board.getType(color, KING);
	if (!king) return true;

	return isAttacked(oppositeColor(color), Bitboard::bitScanForward(king), board);
}

StackVector<std::pair<byte, Piece>, 16> Player::getAllOwnedPieces(const Color color, const Board &board)
{
	StackVector<std::pair<byte, Piece>, 16> pieces;

	for (byte sq = 0u; sq < 64u; ++sq)
		if (const Piece &piece = board.getPiece(sq); piece && piece.isWhite == color)
			pieces.emplace_back(sq, piece);

	return pieces;
}
