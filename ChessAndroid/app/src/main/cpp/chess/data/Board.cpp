#include "Board.h"

#include <algorithm>

#include "../BoardManager.h"
#include "pieces/Piece.h"
#include "pieces/Evaluation.h"
#include "Move.h"
#include "minimax/MiniMax.h"
#include "minimax/Hash.h"

Board::Board(const Board &board)
{
	std::copy(&board.data[0][0], &board.data[0][0] + 64, &data[0][0]);
	hash = board.hash;
	whiteCastled = board.whiteCastled;
	blackCastled = board.blackCastled;
}

Board &Board::operator=(const Board &other)
{
	if (this != &other)
	{
		std::copy(&other.data[0][0], &other.data[0][0] + 64, &data[0][0]);
		hash = other.hash;
		whiteCastled = other.whiteCastled;
		blackCastled = other.blackCastled;
	}
	return *this;
}

Piece &Board::operator[](const Pos &pos)
{
	return data[pos.x][pos.y];
}

const Piece &Board::operator[](const Pos &pos) const
{
	return data[pos.x][pos.y];
}

void Board::initDefaultBoard()
{
	for (short i = 0; i < 8; i++)
		data[i].fill(Piece());

	for (short i = 0; i < 8; i++)
		data[i][1] = Piece(Piece::Type::PAWN, true);

	for (short i = 0; i < 8; i++)
		data[i][6] = Piece(Piece::Type::PAWN, false);

	data[1][0] = Piece(Piece::Type::KNIGHT, true);
	data[6][0] = Piece(Piece::Type::KNIGHT, true);
	data[1][7] = Piece(Piece::Type::KNIGHT, false);
	data[6][7] = Piece(Piece::Type::KNIGHT, false);

	data[2][0] = Piece(Piece::Type::BISHOP, true);
	data[5][0] = Piece(Piece::Type::BISHOP, true);
	data[2][7] = Piece(Piece::Type::BISHOP, false);
	data[5][7] = Piece(Piece::Type::BISHOP, false);

	data[0][0] = Piece(Piece::Type::ROOK, true);
	data[7][0] = Piece(Piece::Type::ROOK, true);
	data[0][7] = Piece(Piece::Type::ROOK, false);
	data[7][7] = Piece(Piece::Type::ROOK, false);

	data[3][0] = Piece(Piece::Type::QUEEN, true);
	data[3][7] = Piece(Piece::Type::QUEEN, false);

	data[4][0] = Piece(Piece::Type::KING, true);
	data[4][7] = Piece(Piece::Type::KING, false);

	hash = Hash::compute(*this);
}

std::unordered_map<Pos, Piece> Board::getAllPieces() const
{
	std::unordered_map<Pos, Piece> map;
	map.reserve(32);

	for (byte i = 0; i < 8; i++)
		for (byte j = 0; j < 8; j++)
			if (data[i][j])
				map[Pos(i, j)] = data[i][j];

	return map;
}

std::vector<Move> Board::listValidMoves(const bool isWhite) const
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	std::vector<Move> moves;
	moves.reserve(93);

	for (const auto &pair : pieces)
	{
		const auto &startPos = pair.first;
		const auto possibleMoves = pair.second.getPossibleMoves(startPos, *this);

		for (const auto &destPos : possibleMoves)
		{
			if ((*this)[destPos].type == Piece::Type::KING)
				continue;

			Board board(*this);
			const auto state = BoardManager::movePieceInternal(startPos, destPos, board);

			if ((isWhite && state == GameState::WHITE_IN_CHESS) ||
				(!isWhite && state == GameState::BLACK_IN_CHESS))
				continue;

			if (state == GameState::NONE)
				moves.emplace_back(startPos, destPos, std::move(board), Evaluation::evaluate(board));
			else if (state == GameState::DRAW)
				moves.emplace_back(startPos, destPos, std::move(board), 0);
			else if (state == GameState::WINNER_WHITE)
				moves.emplace_back(startPos, destPos, std::move(board), MiniMax::VALUE_MAX);
			else if (state == GameState::WINNER_BLACK)
				moves.emplace_back(startPos, destPos, std::move(board), MiniMax::VALUE_MIN);

			++BoardManager::boardsEvaluated;
		}
	}

	std::sort(moves.begin(), moves.end());

	return moves;
}
