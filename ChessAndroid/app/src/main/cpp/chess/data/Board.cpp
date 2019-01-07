#include "Board.h"

#include "pieces/Piece.h"
#include "../minimax/Hash.h"

Board::Board(const Board &board)
{
	std::copy(board.data.begin(), board.data.end(), data.begin());
	hash = board.hash;
	whiteCastled = board.whiteCastled;
	blackCastled = board.blackCastled;
}

Board &Board::operator=(const Board &other)
{
	if (this != &other)
	{
		std::copy(other.data.begin(), other.data.end(), data.begin());
		hash = other.hash;
		whiteCastled = other.whiteCastled;
		blackCastled = other.blackCastled;
		state = GameState::NONE;
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

bool Board::operator<(const Board& other) const
{
	return value < other.value;
}

void Board::initDefaultBoard()
{
	for (short x = 0; x < 8; x++)
		data[x].fill(Piece());

	for (short x = 0; x < 8; x++)
		data[x][1] = Piece(Piece::Type::PAWN, true);

	for (short x = 0; x < 8; x++)
		data[x][6] = Piece(Piece::Type::PAWN, false);

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

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (data[x][y])
				map[Pos(x, y)] = data[x][y];

	return map;
}
