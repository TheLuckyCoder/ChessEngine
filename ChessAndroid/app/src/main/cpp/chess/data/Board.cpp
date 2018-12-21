#include "Board.h"

#include <algorithm>

#include "../BoardManager.h"
#include "pieces/Piece.h"
#include "Move.h"
#include "minimax/MiniMax.h"
#include "minimax/Hash.h"

Board::Board(Board &&board) noexcept
{
	std::move(&board.data[0][0], &board.data[0][0] + 64, &data[0][0]);
	hash = board.hash;
}

Board::Board(const Board &board)
{
	std::copy(&board.data[0][0], &board.data[0][0] + 64, &data[0][0]);
	hash = board.hash;
}

void Board::initDefaultBoard()
{
	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (data[i][j])
				data[i][j] = Piece();

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

Board &Board::operator=(Board &&other) noexcept
{
	if (this != &other)
	{
		std::move(&other.data[0][0], &other.data[0][0] + 64, &data[0][0]);
		hash = other.hash;
	}
	return *this;
}

Board &Board::operator=(const Board &other)
{
	if (this != &other)
	{
		std::copy(&other.data[0][0], &other.data[0][0] + 64, &data[0][0]);
		hash = other.hash;
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

int Board::evaluate() const
{
	int value = 0;

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (data[i][j])
				value += data[i][j].getPoints(i, j);

	return value;
}

std::unordered_map<Pos, Piece> Board::getAllPieces() const
{
	std::unordered_map<Pos, Piece> map;
	map.reserve(32);

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (data[i][j])
				map[Pos(i, j)] = data[i][j];

	return map;
}

std::vector<Move> Board::listAllMoves(const bool isWhite) const
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	std::vector<Move> boards;
	boards.reserve(pieces.size() * 3);

	for (auto &pair : pieces)
	{
		const auto &startPos = pair.first;
		const auto moves = pair.second.getPossibleMoves(startPos, *this);

		for (auto &destPos : moves)
		{
			auto &cache = BoardManager::cache;

			Board board(*this);
			const GameState state = BoardManager::movePieceInternal(startPos, destPos, board);

			board.hash = Hash::compute(board.hash, startPos, destPos, board[startPos], board[destPos]);

			int evaluation;
			if (const auto result = cache.get(board.hash, evaluation); result)
			{
				boards.emplace_back(startPos, destPos, std::move(board), evaluation);
				continue;
			}

			if (state == GameState::NONE)
				boards.emplace_back(startPos, destPos, std::move(board), board.evaluate());
			else if (state == GameState::WINNER_WHITE && isWhite) // TODO
				boards.emplace_back(startPos, destPos, std::move(board), MiniMax::VALUE_MAX);
			else
				boards.emplace_back(startPos, destPos, std::move(board), MiniMax::VALUE_MIN);
		}
	}

	std::sort(boards.begin(), boards.end());

	return boards;
}
