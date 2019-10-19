#include "Board.h"

#include "../algorithm/Hash.h"
#include "../algorithm/Evaluation.h"

Piece &Board::operator[](const Pos &pos) noexcept
{
	return data[pos.x][pos.y];
}

const Piece &Board::operator[](const Pos &pos) const noexcept
{
	return data[pos.x][pos.y];
}

bool Board::operator<(const Board &other) const noexcept
{
	return score < other.score;
}

bool Board::operator>(const Board &other) const noexcept
{
	return score > other.score;
}

/*
 * Returns Piece::EMPTY if the x or y parameters are wrong
 */
const Piece &Board::at(const byte x, const byte y) const noexcept
{
	if (x < 8 && y < 8)
		return data[x][y];
	return Piece::EMPTY;
}

void Board::initDefaultBoard() noexcept
{
	std::memset(&data, 0, sizeof(data));
	npm = 0;

	for (auto &x : data)
		x[1] = Piece(Type::PAWN, true);

	for (auto &x : data)
		x[6] = Piece(Type::PAWN, false);

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
	whiteCastled = blackCastled = false;
	whiteToMove = true;
	state = State::NONE;
	score = 0;
	isPromotion = isCapture = false;
	enPassantPos = Pos();

	constexpr byte whiteKingLocation = Pos(4, 0).toSquare();
	constexpr byte blackKingLocation = Pos(4, 7).toSquare();
	kingSquare[1] = whiteKingLocation;
	kingSquare[0] = blackKingLocation;

	memset(pieces, 0, sizeof(U64) * 2);

	for (byte x = 0u; x < 8u; x++)
	{
		for (byte y = 0u; y < 8u; y++)
		{
			const Piece &piece = getPiece(x, y);
			if (piece)
				pieces[piece.isWhite] |= Pos(x, y).toBitboard();
		}
	}
}

void Board::updateState() noexcept
{
	state = State::NONE;
	if (Player::onlyKingsLeft(*this))
	{
		state = State::DRAW;
		return;
	}

	const bool whiteInCheck = Player::isInCheck(true, *this);
    const bool blackInCheck = Player::isInCheck(false, *this);

	if (whiteInCheck && blackInCheck)
    {
        state = State::INVALID;
        return;
    }

	if (whiteInCheck)
        state = State::WHITE_IN_CHECK;
	else if (blackInCheck)
        state = State::BLACK_IN_CHECK;

	if (whiteToMove)
	{
		if (Player::hasNoValidMoves(true, *this))
			state = whiteInCheck ? State::WINNER_BLACK : State::DRAW;
	}
	else
    {
        if (Player::hasNoValidMoves(false, *this))
            state = blackInCheck ? State::WINNER_WHITE : State::DRAW;
    }
}

Phase Board::getPhase() const noexcept
{
	constexpr short midGameLimit = 15258, endGameLimit = 3915;

	const int limit = std::max(endGameLimit, std::min(npm, midGameLimit));
	return static_cast<Phase>(((limit - endGameLimit) * 128) / (midGameLimit - endGameLimit));
}

StackVector<std::pair<Pos, Piece>, 32> Board::getAllPieces() const noexcept
{
	StackVector<std::pair<Pos, Piece>, 32> pieces;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const Piece &piece = getPiece(x, y))
				pieces.emplace_back(Pos(x, y), piece);

	return pieces;
}

StackVector<Board, 50> Board::listQuiescenceMoves(const bool isWhite) const noexcept
{
	const auto pieces = Player::getAllOwnedPieces(isWhite, *this);
	StackVector<Board, 50> moves;

	for (const auto &pair : pieces)
	{
		const Pos &startPos = pair.first;
		const Piece &selectedPiece = pair.second;
		const auto possibleMoves = selectedPiece.getPossibleCaptures(startPos, *this);

		for (const Pos &destPos : possibleMoves)
		{
			if (moves.size() == 50)
				break; // Just to make sure this won't cause any problems

			const Piece &destPiece = (*this)[destPos];
			if (!destPiece || destPiece.type == Type::KING)
				continue;

			Board board = *this;
			BoardManager::movePieceInternal(startPos, destPos, board);

			if (board.state == State::INVALID)
                continue;
			if (isWhite && (board.state == State::WHITE_IN_CHECK || board.state == State::WINNER_BLACK))
				continue;
			if (!isWhite && (board.state == State::BLACK_IN_CHECK || board.state == State::WINNER_WHITE))
				continue;

			board.score = Evaluation::simpleEvaluation(board);

			moves.push_back(board);
		}
	}

	if (isWhite)
		std::sort(moves.begin(), moves.end(), std::greater<>());
	else
		std::sort(moves.begin(), moves.end());

	return moves;
}
