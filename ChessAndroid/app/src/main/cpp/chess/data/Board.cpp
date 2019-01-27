#include "Board.h"

#include "pieces/Piece.h"
#include "../minimax/Hash.h"

Board::Board(const Board &board)
{
	std::memcpy(&data, &board.data, sizeof(data));
	hash = board.hash;
	whiteCastled = board.whiteCastled;
	blackCastled = board.blackCastled;
	state = board.state;
}

Board &Board::operator=(const Board &other)
{
	if (this != &other)
	{
		std::memcpy(&data, &other.data, sizeof(data));
		hash = other.hash;
		whiteCastled = other.whiteCastled;
		blackCastled = other.blackCastled;
		state = other.state;
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
	whiteCastled = false;
	blackCastled = false;
	state = GameState::NONE;
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

StackVector<Board, 90> Board::listValidMovesQ(const bool isWhite) const
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	StackVector<Board, 90> moves;

	for (const auto &pair : pieces)
	{
		const auto &startPos = pair.first;
		const auto possibleMoves = pair.second.getPossibleMoves(startPos, *this);

		for (const auto &destPos : possibleMoves)
		{
			if (const auto &piece = (*this)[destPos]; !piece && piece.type == Piece::Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(startPos, destPos, board);

			if ((isWhite && board.state == GameState::WHITE_IN_CHESS) ||
				(!isWhite && board.state == GameState::BLACK_IN_CHESS))
				continue;

			moves.push_back(std::move(board));
		}
	}

	std::sort(moves.begin(), moves.end());

	return moves;
}
