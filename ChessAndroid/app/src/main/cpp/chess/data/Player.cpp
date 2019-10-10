#include "Player.h"

#include "Board.h"
#include "Piece.h"
#include "../algorithm/MoveGen.h"
#include "../BoardManager.h"

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

bool Player::isInCheck(bool isWhite, const Board &board)
{
	const U64 king = Bitboard::indexedPos[board.kingSquare[isWhite]];
	bool check = false;

	MoveGen<CAPTURES, false>::forEachAttack(!isWhite, board, [&] (const U64 attacks) -> bool {
		if (king & attacks)
			check = true;
		return check;
	});

	return check;
}

StackVector<std::pair<Pos, Piece>, 16> Player::getAllOwnedPieces(const bool isWhite, const Board &board)
{
	StackVector<std::pair<Pos, Piece>, 16> pieces;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece && piece.isWhite == isWhite)
				pieces.emplace_back(Pos(x, y), piece);

	return pieces;
}
