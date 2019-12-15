#include "Player.h"

#include "../data/Board.h"
#include "MoveGen.h"

bool Player::isAttacked(const Color colorAttacking, const byte targetSquare, const Board &board)
{
    if (board.getType(colorAttacking, PAWN) & PieceAttacks::getPawnAttacks(~colorAttacking, targetSquare))
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
		if (piece && piece.type() != PieceType::KING)
			return false;

	return true;
}

bool Player::hasNoValidMoves(const Color color, const Board &board)
{
	for (byte startSq = 0u; startSq < SQUARE_NB; ++startSq)
	{
		const Piece &attacker = board.getPiece(startSq);
		if (!attacker || attacker.color() != board.colorToMove)
			continue;

		U64 possibleMoves = attacker.getPossibleMoves(startSq, board);
		possibleMoves &= ~board.getType(~color, KING);

		while (possibleMoves)
		{
			const byte destSq = Bitboard::findNextSquare(possibleMoves);

			Board tempBoard = board;
			tempBoard.doMove(startSq, destSq, false);

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
	if (!king) return false;

	return isAttacked(~color, Bitboard::bitScanForward(king), board);
}

AttacksMap Player::getAttacksPerColor(const bool white, const Board &board)
{
	AttacksMap attacks{};

	for (byte startSq = 0u; startSq < SQUARE_NB; ++startSq)
	{
		const Piece &piece = board.getPiece(startSq);
		if (piece && piece.color() == white)
		{
			const Color color = piece.color();
			U64 moves{};
			using Generator = MoveGen<ATTACKS_DEFENSES>;

			switch (piece.type())
			{
				case PieceType::PAWN:
					moves = Generator::getPawnMoves(color, startSq, board);
					break;
				case PieceType::KNIGHT:
					moves = Generator::getKnightMoves(color, startSq, board);
					break;
				case PieceType::BISHOP:
					moves = Generator::getBishopMoves(color, startSq, board);
					break;
				case PieceType::ROOK:
					moves = Generator::getRookMoves(color, startSq, board);
					break;
				case PieceType::QUEEN:
					moves = Generator::getQueenMoves(color, startSq, board);
					break;
				case PieceType::KING:
					moves = Generator::getKingMoves(color, startSq, board);
					break;
				default:
					break;
			}

			while (moves)
			{
				const byte destSq = Bitboard::findNextSquare(moves);
				attacks.map[destSq]++;
				attacks.board[piece.color()][piece.type() - 1u] |= Bitboard::getSquare64(destSq);
			}
		}
	}

	return attacks;
}
