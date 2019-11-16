#include "Player.h"

#include "Board.h"
#include "Piece.h"
#include "../algorithm/MoveGen.h"
#include "../BoardManager.h"

bool Player::isAttacked(const bool whiteAttacking, const byte targetSquare, const Board &board)
{
    if (const U64 pawns = board.pawns[whiteAttacking];
		PieceAttacks::getPawnAttacks(!whiteAttacking, targetSquare) & pawns)
		return true;
    
    if (const U64 knights = board.knights[whiteAttacking];
		PieceAttacks::getKnightAttacks(targetSquare) & knights)
		return true;

    if (Bitboard::shiftedBoards[board.kingSquare[whiteAttacking]] & PieceAttacks::getKingAttacks(targetSquare))
		return true;

    const U64 occupied = board.allPieces[0] | board.allPieces[1];

	if (const U64 bishopsQueens = board.bishops[whiteAttacking] | board.queens[whiteAttacking];
		PieceAttacks::getBishopAttacks(targetSquare, occupied) & bishopsQueens)
		return true;

    if (const U64 rooksQueens = board.rooks[whiteAttacking] | board.queens[whiteAttacking];
    	PieceAttacks::getRookAttacks(targetSquare, occupied) & rooksQueens)
		return true;

	return false;
}

bool Player::onlyKingsLeft(const Board &board)
{
	if (board.npm != 0)
		return false;

	for (const auto &x : board.data)
		for (const Piece &piece : x)
			if (piece && piece.type != Type::KING)
				return false;

	return true;
}

bool Player::hasNoValidMoves(const bool isWhite, const Board &board)
{
	const auto pieces = getAllOwnedPieces(isWhite, board);

	for (const auto &pair : pieces)
	{
		const Pos &startPos = pair.first;
		const Piece &selectedPiece = pair.second;
		const auto possibleMoves = selectedPiece.getPossibleMoves(startPos, board);

		for (const Pos &destPos : possibleMoves)
		{
			if (board[destPos].type == Type::KING)
				continue;

			Board newBoard = board;
			BoardManager::movePieceInternal(startPos, destPos, newBoard, false);

			if (isInCheck(isWhite, newBoard))
				continue;

			return false;
		}
	}

	return true;
}

bool Player::isInCheck(const bool isWhite, const Board &board)
{
	const U64 king = Bitboard::shiftedBoards[board.kingSquare[isWhite]];
	bool check = false;

	MoveGen<CAPTURES, false>::forEachAttack(!isWhite, board, [&] (const U64 attacks) -> bool {
		check = static_cast<bool>(king & attacks);
		return check;
	});
	return check;
}

StackVector<std::pair<Pos, Piece>, 16> Player::getAllOwnedPieces(const bool isWhite, const Board &board)
{
	StackVector<std::pair<Pos, Piece>, 16> pieces;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const Piece &piece = board.getPiece(x, y); piece && piece.isWhite == isWhite)
				pieces.emplace_back(Pos(x, y), piece);

	return pieces;
}
