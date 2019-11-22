#include "Board.h"

#include "../algorithm/Hash.h"
#include "../persistence/FenParser.h"

void Board::initDefaultBoard() noexcept
{
	setToFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Board::setToFen(const std::string &fen)
{
	FenParser parser(*this);
	parser.parseFen(fen);
}

bool Board::canCastleKs(const Color color) const noexcept
{
	return castlingRights & (color == BLACK ? CASTLE_BLACK_KING : CASTLE_WHITE_KING);
}

bool Board::canCastleQs(const Color color) const noexcept
{
	return castlingRights & (color == BLACK ? CASTLE_BLACK_QUEEN : CASTLE_WHITE_QUEEN);
}

bool Board::canCastleAnywhere(const Color color) const noexcept
{
	return castlingRights & (color == BLACK ? CASTLE_BLACK : CASTLE_WHITE);
}

bool Board::isCastled(const Color color) const noexcept
{
	return castlingRights & (color == BLACK ? CASTLED_BLACK : CASTLED_WHITE);
}

Piece &Board::getPiece(const byte squareIndex) noexcept
{
	return data[squareIndex];
}

const Piece &Board::getPiece(const byte squareIndex) const noexcept
{
	return data[squareIndex];
}

Piece &Board::getPiece(const byte x, const byte y) noexcept
{
	return data[Pos(x, y).toSquare()];
}

const Piece &Board::getPiece(const byte x, const byte y) const noexcept
{
	return data[Pos(x, y).toSquare()];
}

/*
 * Returns Piece::EMPTY if the x or y parameters are wrong
 */
const Piece &Board::at(const byte x, const byte y) const noexcept
{
	const Pos pos(x, y);
	if (pos.isValid())
		return data[pos.toSquare()];
	return Piece::EMPTY;
}

U64 &Board::getType(const Color color, const PieceType type) noexcept
{
	return pieces[color][type];
}

U64 Board::getType(const Color color, const PieceType type) const noexcept
{
	return pieces[color][type];
}

Piece &Board::operator[](const Pos &pos) noexcept
{
	return data[pos.toSquare()];
}

const Piece &Board::operator[](const Pos &pos) const noexcept
{
	return data[pos.toSquare()];
}

bool Board::operator<(const Board &other) const noexcept
{
	return score < other.score;
}

bool Board::operator>(const Board &other) const noexcept
{
	return score > other.score;
}

void Board::updateState() noexcept
{
	state = State::NONE;
	if (Player::onlyKingsLeft(*this) || halfMoveClock == 50u)
	{
		state = State::DRAW;
		return;
	}

	const bool whiteInCheck = Player::isInCheck(WHITE, *this);
    const bool blackInCheck = Player::isInCheck(BLACK, *this);

	if (whiteInCheck && blackInCheck)
    {
        state = State::INVALID;
        return;
    }

	if (whiteInCheck)
        state = State::WHITE_IN_CHECK;
	else if (blackInCheck)
        state = State::BLACK_IN_CHECK;

	if (colorToMove)
	{
		if (Player::hasNoValidMoves(WHITE, *this))
			state = whiteInCheck ? State::WINNER_BLACK : State::DRAW;
	}
	else
    {
        if (Player::hasNoValidMoves(BLACK, *this))
            state = blackInCheck ? State::WINNER_WHITE : State::DRAW;
    }
}

Phase Board::getPhase() const noexcept
{
	constexpr short midGameLimit = 15258;
	constexpr short endGameLimit = 3915;

	const int limit = std::max(endGameLimit, std::min(npm, midGameLimit));
	return static_cast<Phase>(((limit - endGameLimit) * 128) / (midGameLimit - endGameLimit));
}

bool Board::hasValidState() const noexcept
{
    if (state == State::INVALID)
        return false;
    if (colorToMove && (state == State::WHITE_IN_CHECK || state == State::WINNER_BLACK))
        return false;
    if (!colorToMove && (state == State::BLACK_IN_CHECK || state == State::WINNER_WHITE))
        return false;

    return true;
}

std::vector<std::pair<Pos, Piece>> Board::getAllPieces() const
{
	std::vector<std::pair<Pos, Piece>> pieces;
	pieces.reserve(32);

	for (byte square = 0u; square < 64u; ++square)
		if (const Piece &piece = data[square])
			pieces.emplace_back(Pos(square), piece);

	return pieces;
}

std::vector<Board> Board::listQuiescenceMoves() const
{
	const auto pieces = Player::getAllOwnedPieces(colorToMove, *this);
	std::vector<Board> moves;
	moves.reserve(50);

	for (const auto &pair : pieces)
	{
		const byte startSq = pair.first;
		const Piece &selectedPiece = pair.second;
		U64 possibleMoves = selectedPiece.getPossibleCaptures(startSq, *this);

		while (possibleMoves)
		{
			const byte destSq = Bitboard::findNextSquare(possibleMoves);
			const Piece &destPiece = getPiece(destSq);
			if (!destPiece || destPiece.type == PieceType::KING)
				continue;

			Board board = *this;
			board.doMove(startSq, destSq);

			if (!board.hasValidState())
                continue;

			board.score = Evaluation::simpleEvaluation(board);

			moves.push_back(board);
		}
	}

	if (colorToMove)
		std::sort(moves.begin(), moves.end(), std::greater<>());
	else
		std::sort(moves.begin(), moves.end());

	return moves;
}

void Board::doMove(const byte startSq, const byte destSq, const bool updateState) noexcept
{
	const Color whiteIsMoving = colorToMove;
	const Piece &startPiece = getPiece(startSq);
	const Piece &destPiece = getPiece(destSq);
	const U64 startBb = Bitboard::shiftedBoards[startSq];
	const U64 destBb = Bitboard::shiftedBoards[destSq];

	score = 0;
	state = State::NONE;
	colorToMove = oppositeColor(colorToMove);
	isPromotion = isCapture = false;
	++halfMoveClock;

	getType(whiteIsMoving, startPiece.type) &= ~startBb;
	getType(oppositeColor(whiteIsMoving), destPiece.type) &= ~destBb;
	getType(whiteIsMoving, startPiece.type) |= destBb;

	Hash::flipSide(zKey);
	Hash::makeMove(zKey, startSq, destSq, startPiece, destPiece);

	if (startPiece.type == PieceType::PAWN)
	{
		movePawn(startSq, destSq);
		halfMoveClock = 0u;
	}
	else
	{
		enPassant = 0ull;
		if (startPiece.type == PieceType::ROOK)
			moveRook(startSq);
		else if (startPiece.type == PieceType::KING)
			moveKing(startPiece, startSq, destSq);
	}

	if (destPiece)
	{
		npm -= Evaluation::getPieceValue(destPiece.type);
		isCapture = true;
		halfMoveClock = 0u;
	}

	getPiece(destSq) = startPiece;
	getPiece(startSq) = Piece();
	updateNonPieceBitboards();

	if (updateState)
		this->updateState();
}

bool Board::movePawn(const byte startSq, const byte destSq)
{
	Piece &pawn = getPiece(startSq);

	if (row(destSq) == 0u || row(destSq) == 7u)
	{
		const PieceType newPieceType = PieceType::QUEEN;
		pawn.type = newPieceType;
		isPromotion = true;

		Hash::promotePawn(zKey, destSq, toColor(pawn.isWhite), newPieceType);
		const U64 destBb = Bitboard::shiftedBoards[destSq];

		getType(toColor(pawn.isWhite), PieceType::PAWN) &= ~destBb;
		getType(toColor(pawn.isWhite), newPieceType) |= destBb;
		return true;
	}

	if (destSq == enPassant) {
		isCapture = true;

		Pos capturedPos(enPassant);
		capturedPos.y += static_cast<byte>(pawn.isWhite ? -1 : 1);
		Piece &capturedPiece = getPiece(capturedPos.toSquare());

		// Remove the captured Pawn
		Hash::xorPiece(zKey, capturedPos.toSquare(), capturedPiece);
		npm -= Evaluation::getPieceValue(PieceType::PAWN);
		capturedPiece = Piece();
		getType(toColor(capturedPiece.isWhite), capturedPiece.type);
		return true;
	}

	const int distance = static_cast<int>(row(destSq)) - static_cast<int>(row(startSq));
	if (distance == 2 || distance == -2)
	{
		Pos newEnPassant(destSq);
		newEnPassant.y -= static_cast<byte>(distance / 2);
		enPassant = newEnPassant.toSquare();
	} else
		enPassant = 0ull;

	return false;
}

void Board::moveRook(const byte startSq)
{
	const bool isPieceWhite = getPiece(startSq).isWhite;

	if (col(startSq) == 0u)
		castlingRights &= isPieceWhite ? ~CASTLE_WHITE_QUEEN : CASTLE_BLACK_QUEEN;
	else if (col(startSq) == 7u)
		castlingRights &= isPieceWhite ? ~CASTLE_WHITE_KING : CASTLE_BLACK_KING;
}

void Board::moveKing(const Piece &king, const byte startSq, const byte destSq)
{
	if (!(king & (CastlingRights::CASTLE_WHITE | CastlingRights::CASTLE_BLACK))) return;

	const Color color = toColor(king.isWhite);
	bool castled = false;

	if (col(destSq) == 6u)
	{
		constexpr byte startX = 7u;
		const byte y = row(startSq);

		Piece &rook = getPiece(startX, y);
		if (rook.type == PieceType::ROOK && king.isSameColor(rook) && canCastleKs(color))
		{
			constexpr byte destX = 5;
			getPiece(destX, y) = rook;
			rook = Piece::EMPTY;

			getType(color, ROOK) &= ~Pos(destX, y).toBitboard();
			getType(color, ROOK) &= ~Pos(startX, y).toBitboard();

			castled = true;
			Hash::makeMove(zKey, Pos(startX, y).toSquare(), Pos(destX, y).toSquare(), rook);
		}
	}
	else if (col(destSq) == 2u)
	{
		constexpr byte startX = 0u;
		const byte y = row(startSq);

		Piece &rook = getPiece(startX, y);
		if (rook.type == PieceType::ROOK && king.isSameColor(rook) && canCastleQs(color))
		{
			constexpr byte destX = 3u;
			getPiece(destX, y) = rook;
			rook = Piece::EMPTY;

			getType(color, ROOK) &= ~Pos(destX, y).toBitboard();
			getType(color, ROOK) &= ~Pos(startX, y).toBitboard();

			castled = true;
			Hash::makeMove(zKey, Pos(startX, y).toSquare(), Pos(destX, y).toSquare(), rook);
		}
	}

	if (castled)
	{
		Hash::xorCastlingRights(zKey, static_cast<CastlingRights>(castlingRights));
		if (king.isWhite)
		{
			castlingRights &= ~CASTLE_WHITE;
			castlingRights |= CASTLED_WHITE;
		} else {
			castlingRights &= ~CASTLE_BLACK;
			castlingRights |= CASTLED_BLACK;
		}
		Hash::xorCastlingRights(zKey, static_cast<CastlingRights>(castlingRights));
	}
}

void Board::updateNonPieceBitboards()
{
	allPieces[BLACK] = 0ull;
	allPieces[WHITE] = 0ull;

	for (byte i = PAWN; i <= KING; ++i)
	{
		allPieces[BLACK] |= getType(BLACK, static_cast<PieceType>(i));
		allPieces[WHITE] |= getType(WHITE, static_cast<PieceType>(i));
	}

	occupied = allPieces[BLACK] | allPieces[WHITE];
}
