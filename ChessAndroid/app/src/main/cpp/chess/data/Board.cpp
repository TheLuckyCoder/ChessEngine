#include "Board.h"

#include <algorithm>
#include <iterator>

#include "../BoardManager.h"
#include "pieces/Pieces.h"
#include "Move.h"
#include "player/MiniMax.h"

Board::Board()
{
	memset(data, 0, sizeof(Piece*) * 64);
}

Board::Board(const Board &board)
{
	auto makeCopy = [](Piece *piece) -> Piece* {
		switch (piece->type)
		{
		case Piece::Type::PAWN:
			return new PawnPiece(piece);
		case Piece::Type::KNIGHT:
			return new KnightPiece(piece);
		case Piece::Type::BISHOP:
			return new BishopPiece(piece);
		case Piece::Type::ROOK:
			return new RookPiece(piece);
		case Piece::Type::QUEEN:
			return new QueenPiece(piece);
		case Piece::Type::KING:
			return new KingPiece(piece);
		}
	};

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (board.data[i][j])
				data[i][j] = makeCopy(board.data[i][j]);
}

Board::Board(Board &&board)
{
    std::move(&board.data[0][0], &board.data[0][0] + 64, &data[0][0]);
}

Board::~Board()
{
	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (data[i][j])
				delete data[i][j];
}

void Board::initDefaultBoard()
{
	memset(data, 0, sizeof(Piece*) * 64);

	for (short i = 0; i < 8; i++)
		data[i][1] = new PawnPiece(true);
		
	for (short i = 0; i < 8; i++)
		data[i][6] = new PawnPiece(false);

	data[1][0] = new KnightPiece(true);
	data[6][0] = new KnightPiece(true);
	data[1][7] = new KnightPiece(false);
	data[6][7] = new KnightPiece(false);

	data[2][0] = new BishopPiece(true);
	data[5][0] = new BishopPiece(true);
	data[2][7] = new BishopPiece(false);
	data[5][7] = new BishopPiece(false);
	
	data[0][0] = new RookPiece(true);
	data[7][0] = new RookPiece(true);
	data[0][7] = new RookPiece(false);
	data[7][7] = new RookPiece(false);

	data[3][0] = new QueenPiece(true);
	data[3][7] = new QueenPiece(false);

	data[4][0] = new KingPiece(true);
	data[4][7] = new KingPiece(false);
}

Board &Board::operator=(Board &&other)
{
	if (this != &other)
	{
		for (short i = 0; i < 8; i++)
			for (short j = 0; j < 8; j++)
				if (data[i][j])
				{
					delete data[i][j];
					data[i][j] = other.data[i][j];;
				}
	}
	return *this;
}

Piece *Board::operator[](const Pos &pos)
{
	return data[pos.x][pos.y];
}

const Piece *Board::operator[](const Pos &pos) const
{
	return data[pos.x][pos.y];
}

int Board::evaluate() const
{
	int value = 0;

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (data[i][j])
				value += data[i][j]->getPoints(i, j);

	return value;
}

std::unordered_map<Pos, Piece*> Board::getAllPieces() const
{
	std::unordered_map<Pos, Piece*> map;
	map.reserve(32);

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
			if (data[i][j])
				map[Pos(i, j)] = data[i][j];

	return map;
}

std::vector<Move> Board::listAllMoves(const bool isWhite) const
{
	const std::unordered_map<Pos, Piece*> pieces = Player::getAllOwnedPieces(isWhite, *this);
	std::vector<Move> boards;
	boards.reserve(pieces.size() * 3);

	for (auto &pair : pieces)
	{
		const auto &startPos = pair.first;
		const auto moves = pair.second->getPossibleMoves(startPos, *this);

		for (auto &destPos : moves)
		{
			auto *board = new Board(*this);
			GameState state = BoardManager::movePieceInternal(startPos, destPos, *board);

			if (state == GameState::NONE)
				boards.emplace_back(startPos, destPos, board);
			else if (state == GameState::WINNER_WHITE && isWhite)
				boards.emplace_back(startPos, destPos, board, MiniMax::VALUE_MAX);
			else
				boards.emplace_back(startPos, destPos, board, MiniMax::VALUE_MIN);
		}
	}

	std::sort(boards.begin(), boards.end());

	return boards;
}
