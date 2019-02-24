#include "Board.h"

#include "../algorithm/Hash.h"

Board::Board(const Board &board) noexcept
{
	std::memcpy(&data, &board.data, sizeof(data));
	hash = board.hash;
	whiteCastled = board.whiteCastled;
	blackCastled = board.blackCastled;
	whiteKingPos = board.whiteKingPos;
	blackKingPos = board.blackKingPos;
	state = board.state;
}

Board &Board::operator=(const Board &other) noexcept
{
	std::memcpy(&data, &other.data, sizeof(data));
	hash = other.hash;
	whiteCastled = other.whiteCastled;
	blackCastled = other.blackCastled;
	whiteKingPos = other.whiteKingPos;
	blackKingPos = other.blackKingPos;
	state = other.state;
	return *this;
}

Piece &Board::operator[](const Pos &pos) noexcept
{
	return data[pos.x][pos.y];
}

const Piece &Board::operator[](const Pos &pos) const noexcept
{
	return data[pos.x][pos.y];
}

bool Board::operator<(const Board& other) const noexcept
{
	return score < other.score;
}

bool Board::operator>(const Board& other) const noexcept
{
	return score > other.score;
}

void Board::initDefaultBoard() noexcept
{
	using Type = Piece::Type;

	for (byte x = 0; x < 8; x++)
		data[x].fill(Piece());

	for (byte x = 0; x < 8; x++)
		data[x][1] = Piece(Type::PAWN, true);

	for (byte x = 0; x < 8; x++)
		data[x][6] = Piece(Type::PAWN, false);

	data[1][0] = Piece(Type::KNIGHT, true);
	data[6][0] = Piece(Type::KNIGHT, true);
	data[1][7] = Piece(Type::KNIGHT, false);
	data[6][7] = Piece(Type::KNIGHT, false);

	data[2][0] = Piece(Type::BISHOP, true);
	data[5][0] = Piece(Type::BISHOP, true);
	data[2][7] = Piece(Type::BISHOP, false);
	data[5][7] = Piece(Type::BISHOP, false);

	data[0][0] = Piece(Type::ROOK, true);
	data[7][0] = Piece(Type::ROOK, true);
	data[0][7] = Piece(Type::ROOK, false);
	data[7][7] = Piece(Type::ROOK, false);

	data[3][0] = Piece(Type::QUEEN, true);
	data[3][7] = Piece(Type::QUEEN, false);

	data[4][0] = Piece(Type::KING, true);
	data[4][7] = Piece(Type::KING, false);

	hash = Hash::compute(*this);
	whiteCastled = false;
	blackCastled = false;
	whiteKingPos = Pos(4, 0).toBitboard();
	blackKingPos = Pos(4, 7).toBitboard();
	state = State::NONE;
}

void Board::updateState() noexcept
{
	state = Player::onlyKingsLeft(*this) ? State::DRAW : State::NONE;

	const bool whiteInChess = Player::isInChess(true, *this);
	if (whiteInChess)
		state = State::WHITE_IN_CHESS;

	if (Player::hasNoValidMoves(true, *this))
		state = whiteInChess ? State::WINNER_BLACK : State::DRAW;
	else
	{
		const bool blackInChess = Player::isInChess(false, *this);
		if (blackInChess)
			state = State::BLACK_IN_CHESS;
		if (Player::hasNoValidMoves(false, *this))
			state = blackInChess ? State::WINNER_WHITE : State::DRAW;
	}
}

StackVector<std::pair<Pos, Piece>, 32> Board::getAllPieces() const noexcept
{
	StackVector<std::pair<Pos, Piece>, 32> pieces;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (data[x][y])
				pieces.emplace_back(Pos(x, y), data[x][y]);

	return pieces;
}

StackVector<Board, 50> Board::listValidCaptures(const bool isWhite) const noexcept
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	StackVector<Board, 50> moves;

	for (const auto &pair : pieces)
	{
		const auto &startPos = pair.first;
		const auto possibleMoves = pair.second.getPossibleCaptures(startPos, *this);

		for (const auto &destPos : possibleMoves)
		{
			if (const auto &piece = (*this)[destPos]; !piece || piece.type == Piece::Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(startPos, destPos, board);

			if (isWhite && (board.state == State::WHITE_IN_CHESS || board.state == State::WINNER_BLACK))
				continue;
			if (!isWhite && (board.state == State::BLACK_IN_CHESS || board.state == State::WINNER_WHITE))
				continue;

			moves.push_back(std::move(board));
		}
	}

	if (isWhite)
		std::sort(moves.begin(), moves.end(), std::greater<>());
	else
		std::sort(moves.begin(), moves.end());

	return moves;
}
