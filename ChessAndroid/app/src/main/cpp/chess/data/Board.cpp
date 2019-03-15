#include "Board.h"

#include "../algorithm/Hash.h"
#include "../algorithm/Evaluation.h"

Board &Board::operator=(const Board &other) noexcept
{
	std::memcpy(this, &other, sizeof(Board));
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
	std::memset(&data, 0, sizeof(data));
	npm = 0;

	for (byte x = 0; x < 8; x++)
		data[x][1] = Piece(Type::PAWN, true);

	for (byte x = 0; x < 8; x++)
		data[x][6] = Piece(Type::PAWN, false);

	npm += 16 * Evaluation::getPieceValue(Type::PAWN);

	data[1][0] = Piece(Type::KNIGHT, true);
	data[6][0] = Piece(Type::KNIGHT, true);
	data[1][7] = Piece(Type::KNIGHT, false);
	data[6][7] = Piece(Type::KNIGHT, false);
	npm += 4 * Evaluation::getPieceValue(Type::KNIGHT);

	data[2][0] = Piece(Type::BISHOP, true);
	data[5][0] = Piece(Type::BISHOP, true);
	data[2][7] = Piece(Type::BISHOP, false);
	data[5][7] = Piece(Type::BISHOP, false);
	npm += 4 * Evaluation::getPieceValue(Type::BISHOP);

	data[0][0] = Piece(Type::ROOK, true);
	data[7][0] = Piece(Type::ROOK, true);
	data[0][7] = Piece(Type::ROOK, false);
	data[7][7] = Piece(Type::ROOK, false);
	npm += 4 * Evaluation::getPieceValue(Type::ROOK);

	data[3][0] = Piece(Type::QUEEN, true);
	data[3][7] = Piece(Type::QUEEN, false);
	npm += 2 * Evaluation::getPieceValue(Type::QUEEN);

	data[4][0] = Piece(Type::KING, true);
	data[4][7] = Piece(Type::KING, false);

	key = Hash::compute(*this);
	whiteCastled = false;
	blackCastled = false;
	whiteKingPos = Pos(4, 0).toBitboard();
	blackKingPos = Pos(4, 7).toBitboard();
	whiteToMove = true;
	state = State::NONE;
	score = 0;
}

void Board::updateState() noexcept
{
	state = State::NONE;
	if (Player::onlyKingsLeft(*this))
	{
		state = State::DRAW;
		return;
	}

	const bool whiteInChess = Player::isInChess(true, *this);
    const bool blackInChess = Player::isInChess(false, *this);

	if (whiteInChess && blackInChess)
    {
        state = State::INVALID;
        return;
    }
	if (whiteInChess)
        state = State::WHITE_IN_CHESS;
	else if (blackInChess)
        state = State::BLACK_IN_CHESS;

	if (whiteToMove)
	{
		if (Player::hasNoValidMoves(true, *this))
			state = whiteInChess ? State::WINNER_BLACK : State::DRAW;
	}
	else
    {
        if (Player::hasNoValidMoves(false, *this))
            state = blackInChess ? State::WINNER_WHITE : State::DRAW;
    }
}

Phase Board::getPhase() const noexcept
{
	constexpr int midGameLimit = 15258, endGameLimit = 3915;

	const int limit = std::max(endGameLimit, std::min(npm, midGameLimit));
	return static_cast<Phase>(((limit - endGameLimit) * 128) / (midGameLimit - endGameLimit));
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
			if (const auto &piece = (*this)[destPos]; !piece || piece.type == Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(startPos, destPos, board);

			if (board.state == State::INVALID)
                continue;
			if (isWhite && (board.state == State::WHITE_IN_CHESS || board.state == State::WINNER_BLACK))
				continue;
			if (!isWhite && (board.state == State::BLACK_IN_CHESS || board.state == State::WINNER_WHITE))
				continue;

			moves.push_back(board);
		}
	}

	if (isWhite)
		std::sort(moves.begin(), moves.end(), std::greater<>());
	else
		std::sort(moves.begin(), moves.end());

	return moves;
}
