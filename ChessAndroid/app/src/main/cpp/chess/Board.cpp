#include "Board.h"

#include "Zobrist.h"
#include "algorithm/Evaluation.h"
#include "persistence/FenParser.h"

void Board::setToStartPos()
{
	setToFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool Board::setToFen(const std::string &fen)
{
	Board copy = *this;
	const bool result = FenParser::parseFen(*this, fen);

	if (!result)
		*this = copy;

	return result;
}

std::string Board::getFen() const
{
	return FenParser::exportToFen(*this);
}

bool Board::canCastle(const Color color) const noexcept
{
	return color == BLACK ? canCastle<BLACK>() : canCastle<WHITE>();
}

Piece &Board::getPiece(const Square square) noexcept
{
	return data[square];
}

Bitboard &Board::getPieces(const Piece piece) noexcept
{
    return pieces[piece.color()][piece.type()];
}

Piece Board::getPiece(const Square square) const noexcept
{
    return data[square];
}

Bitboard Board::getPieces() const noexcept
{
    return occupied;
}

Bitboard Board::getPieces(const PieceType type, const Color color) const noexcept
{
    return pieces[color][type];
}

Bitboard Board::getPieces(const PieceType type) const noexcept
{
	return pieces[BLACK][type] | pieces[WHITE][type];
}

Bitboard Board::getPieces(const Color color) const noexcept
{
    return pieces[color][PieceType::NO_PIECE_TYPE];
}

bool Board::isDrawn() const noexcept
{
	// Fifty Move Rules
	if (fiftyMoveRule > 99)
		return true;

	// Three-fold repetition
	i32 repetitions = 1;
	for (i32 i = historyPly - fiftyMoveRule; i < historyPly - 1; ++i)
		if (zKey == history[i].zKey && ++repetitions == 3)
			return true;

	// Insufficient Material
	// KvK, KvN, KvB, KvNN
	const auto pawns = getPieces(PAWN);
	const auto knight = getPieces(KNIGHT);
	const auto bishop = getPieces(BISHOP);
	const auto rooks = getPieces(ROOK);
	const auto queens = getPieces(QUEEN);

	return (pawns | rooks | queens).empty()
		   && (!(getPieces(WHITE).several()) || !(getPieces(BLACK).several()))
		   && (!((knight | bishop).several()) || (!bishop && knight.popcount() <= 2));
}

Phase Board::getPhase() const noexcept
{
	constexpr short midGameLimit = 15258;
	constexpr short endGameLimit = 3915;

	const short limit = std::max(endGameLimit, std::min(npm, midGameLimit));
	return Phase(((limit - endGameLimit) * 128) / (midGameLimit - endGameLimit));
}

bool Board::makeMove(const Move move) noexcept
{
	assert(!move.empty());
	const Square from = move.from();
	const Square to = move.to();
	const Color side = colorToMove;
	const auto flags = move.flags();
	const u64 posKey = zKey;
	const PieceType movedPiece = move.piece();

	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);
	assert(Piece::isValid(movedPiece));
	assert(getPiece(from).isValid());
	assert(getPiece(from).type() == movedPiece);

	// Handle en passant capture and castling
	if (flags.enPassant())
	{
		const Square capturedSq = toSquare(u8(to) + static_cast<u8>(side ? -8 : 8));
		removePiece(capturedSq);
	} else if (flags.kSideCastle())
	{
		switch (to)
		{
			case SQ_G1:
				movePiece(SQ_H1, SQ_F1);
				break;
			case SQ_G8:
				movePiece(SQ_H8, SQ_F8);
				break;
			default:
				assert(false);
		}

		castlingRights |= (side ? CASTLED_WHITE : CASTLED_BLACK);
	} else if (flags.qSideCastle())
	{
		switch (to)
		{
			case SQ_C1:
				movePiece(SQ_A1, SQ_D1);
				break;
			case SQ_C8:
				movePiece(SQ_A8, SQ_D8);
				break;
			default:
				assert(false);
		}

		castlingRights |= (side ? CASTLED_WHITE : CASTLED_BLACK);
	}

	Zobrist::xorEnPassant(zKey, enPassantSq);
	Zobrist::xorCastlingRights(zKey, CastlingRights(castlingRights));

	// Store the position info in history
	history[historyPly] = { posKey, kingAttackers, move.getContents(), castlingRights, enPassantSq, fiftyMoveRule };

	if (canCastle(side))
	{
		if (movedPiece == ROOK)
		{
			const auto rookFile = Bitboard::fromFile(from);
			if (rookFile == FILE_A)
				castlingRights &= ~(side ? CASTLE_WHITE_QUEEN : CASTLE_BLACK_QUEEN);
			else if (rookFile == FILE_H)
				castlingRights &= ~(side ? CASTLE_WHITE_KING : CASTLE_BLACK_KING);
		} else if (movedPiece == KING)
			// Remove all castling rights if the king is moved
			castlingRights &= ~(side ? CASTLE_WHITE_BOTH : CASTLE_BLACK_BOTH);
	}

	enPassantSq = Square::SQ_NONE;
	++fiftyMoveRule;

	if (const PieceType capturedType = move.capturedPiece();
		capturedType != PieceType::NO_PIECE_TYPE)
	{
		assert(Piece::isValid(capturedType));
		if (capturedType == ROOK && canCastle(~side))
		{
			const auto rookFile = Bitboard::fromFile(to);
			if (rookFile == FILE_A)
				castlingRights &= ~(~side ? CASTLE_WHITE_QUEEN : CASTLE_BLACK_QUEEN);
			else if (rookFile == FILE_H)
				castlingRights &= ~(~side ? CASTLE_WHITE_KING : CASTLE_BLACK_KING);
		}
		removePiece(to);
		fiftyMoveRule = 0;
	}

	++historyPly;
	++ply;

	if (movedPiece == PAWN)
	{
		fiftyMoveRule = 0;

		if (move.flags().doublePawnPush())
		{
			enPassantSq = toSquare(u8(from) + static_cast<u8>(side ? 8 : -8));

			Zobrist::xorEnPassant(zKey, enPassantSq);
			assert(Bitboard::fromRank(enPassantSq) == RANK_3
				|| Bitboard::fromRank(enPassantSq) == RANK_6);
		}
	}

	movePiece(from, to);

	if (move.flags().promotion())
	{
		const PieceType promotedPiece = move.promotedPiece();
		assert(Piece::isValid(promotedPiece));
		assert(promotedPiece != PAWN && promotedPiece != KING);

		removePiece(to); // Remove Pawn
		addPiece(to, { promotedPiece, side });
	}

	colorToMove = ~colorToMove;
	Zobrist::flipSide(zKey);

	updateNonPieceBitboards();

	if (move.capturedPiece() == KING
		|| (side ? generateKingAttackers<WHITE>() : generateKingAttackers<BLACK>()))
	{
		undoMove();
		return false;
	}

	kingAttackers = colorToMove ? generateKingAttackers<WHITE>() : generateKingAttackers<BLACK>();

	return true;
}

void Board::undoMove() noexcept
{
	--historyPly;
	--ply;

	const UndoMove &previousMove = history[historyPly];
	const Move move = previousMove.getMove();
	const Square from = move.from();
	const Square to = move.to();
	const auto flags = move.flags();

	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);

	kingAttackers = previousMove.kingAttackers;
	castlingRights = previousMove.castlingRights;
	fiftyMoveRule = previousMove.fiftyMoveRule;
	enPassantSq = previousMove.enPassantSq;

	colorToMove = ~colorToMove;

	if (flags.enPassant())
	{
		const Square capturesSq = toSquare(u8(to) + static_cast<u8>(colorToMove ? -8 : 8));
		addPiece(capturesSq, { PAWN, ~colorToMove });
	} else if (flags.kSideCastle())
	{
		switch (to)
		{
			case SQ_G1:
				movePiece(SQ_F1, SQ_H1);
				break;
			case SQ_G8:
				movePiece(SQ_F8, SQ_H8);
				break;
			default:
				assert(false);
		}
	} else if (flags.qSideCastle())
	{
		switch (to)
		{
			case SQ_C1:
				movePiece(SQ_D1, SQ_A1);
				break;
			case SQ_C8:
				movePiece(SQ_D8, SQ_A8);
				break;
			default:
				assert(false);
		}
	}

	movePiece(to, from);

	if (const PieceType capturedType = move.capturedPiece();
		capturedType != NO_PIECE_TYPE)
		addPiece(to, Piece(capturedType, ~colorToMove));

	if (flags.promotion())
	{
		removePiece(from);
		addPiece(from, Piece(PAWN, colorToMove));
	}

	zKey = previousMove.zKey;
	updateNonPieceBitboards();
}

void Board::makeNullMove() noexcept
{
	assert(!isSideInCheck());

	++ply;
	history[historyPly++] = { zKey, kingAttackers, {}, castlingRights, enPassantSq, fiftyMoveRule };

	Zobrist::xorEnPassant(zKey, enPassantSq);
	enPassantSq = SQ_NONE;

	colorToMove = ~colorToMove;
	Zobrist::flipSide(zKey);

	kingAttackers = colorToMove ? generateKingAttackers<WHITE>() : generateKingAttackers<BLACK>();
}

void Board::undoNullMove() noexcept
{
	--historyPly;
	--ply;

	const UndoMove &previousMove = history[historyPly];
	zKey = previousMove.zKey;
	kingAttackers = previousMove.kingAttackers;
	castlingRights = previousMove.castlingRights;
	fiftyMoveRule = previousMove.fiftyMoveRule;
	enPassantSq = previousMove.enPassantSq;

    colorToMove = ~colorToMove;
}

bool
Board::isAttackedByAny(const Color attackerColor, const Square targetSquare, const Bitboard blockers) const noexcept
{
    return isAttacked<PAWN>(attackerColor, targetSquare, blockers)
           || isAttacked<KNIGHT>(attackerColor, targetSquare, blockers)
           || isAttacked<KING>(attackerColor, targetSquare, blockers)
           || isAttacked<BISHOP>(attackerColor, targetSquare, blockers)
           || isAttacked<ROOK>(attackerColor, targetSquare, blockers)
           || isAttacked<QUEEN>(attackerColor, targetSquare, blockers);
}

bool Board::isAttackedByAny(const Color attackerColor, const Square targetSquare) const noexcept
{
    return isAttackedByAny(attackerColor, targetSquare, getPieces());
}

bool Board::isSideInCheck() const noexcept
{
	return !kingAttackers.empty();
}

void Board::addPiece(const Square square, const Piece piece) noexcept
{
	assert(piece.isValid());

	Zobrist::xorPiece(zKey, square, piece);
    getPieces(piece).addSquare(square);
	getPiece(square) = piece;
	if (piece.type() != PAWN)
		npm += Evaluation::getPieceValue(piece.type());

	pieceList[piece][pieceCount[piece]++] = square;
}

void Board::movePiece(const Square from, const Square to) noexcept
{
	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);

	const Piece piece = getPiece(from);
	assert(piece.isValid());

	Zobrist::xorPiece(zKey, from, piece);
	getPiece(from) = Piece();
    getPieces(piece).removeSquare(from);

	Zobrist::xorPiece(zKey, to, piece);
	getPiece(to) = piece;
    getPieces(piece).addSquare(to);

    [[maybe_unused]] bool pieceMoved = false;
	for (u8 &sq : pieceList[piece])
	{
		if (sq == from)
		{
			sq = to;
			pieceMoved = true;
			break;
		}
	}
	assert(pieceMoved);
}

void Board::removePiece(const Square square) noexcept
{
	assert(square < SQUARE_NB);

	const Piece piece = getPiece(square);
	assert(piece.isValid());

	Zobrist::xorPiece(zKey, square, piece);
    getPieces(piece).removeSquare(square);
	getPiece(square) = Piece();

	if (piece.type() != PAWN)
		npm -= Evaluation::getPieceValue(piece.type());

	auto &piecesSquares = pieceList[piece];

	u8 pieceIndex = -1;
	for (u8 i = 0; i < pieceCount[piece]; ++i)
	{
		if (piecesSquares[i] == square)
		{
			pieceIndex = i;
			break;
		}
	}

	assert(pieceIndex < SQUARE_NB);

	// Move the last square in this unused index
	piecesSquares[pieceIndex] = piecesSquares[--pieceCount[piece]];
}

void Board::updatePieceList() noexcept
{
	pieceCount.fill({});

	for (u8 sq{}; sq < SQUARE_NB; ++sq)
	{
		const Piece &piece = data[sq];
		if (piece)
		{
			pieceList[piece][pieceCount[piece]++] = sq;

            getPieces(piece).addSquare(toSquare(sq));

			if (piece.type() != PAWN)
				npm += Evaluation::getPieceValue(piece.type());
		}
	}
}

void Board::updateNonPieceBitboards() noexcept
{
	pieces[BLACK][NO_PIECE_TYPE] = getPieces(PAWN, BLACK)
                       | getPieces(KNIGHT, BLACK)
                       | getPieces(BISHOP, BLACK)
                       | getPieces(ROOK, BLACK)
                       | getPieces(QUEEN, BLACK)
                       | getPieces(KING, BLACK);

    pieces[WHITE][NO_PIECE_TYPE] = getPieces(PAWN, WHITE)
                       | getPieces(KNIGHT, WHITE)
                       | getPieces(BISHOP, WHITE)
                       | getPieces(ROOK, WHITE)
                       | getPieces(QUEEN, WHITE)
                       | getPieces(KING, WHITE);

	occupied = pieces[BLACK][NO_PIECE_TYPE] | pieces[WHITE][NO_PIECE_TYPE];
}

std::string Board::toString() const noexcept
{
	std::ostringstream ss;

    static constexpr auto Delimiter = "+---+---+---+---+---+---+---+---+";

	for (u8 sq{}; sq < SQUARE_NB; ++sq)
	{
		if (sq % 8 == 0)
			ss << '\n' << Delimiter << '\n' << '|';

		const Piece piece = data[sq];
		const auto type = piece.type();
		const bool color = piece.color();

		const char c = [&]
		{
			switch (type)
			{
				case NO_PIECE_TYPE:
					return ' ';
				case PAWN:
					return color ? 'P' : 'p';
				case KNIGHT:
					return color ? 'N' : 'n';
				case BISHOP:
					return color ? 'B' : 'b';
				case ROOK:
					return color ? 'R' : 'r';
				case QUEEN:
					return color ? 'Q' : 'q';
				case KING:
					return color ? 'K' : 'k';
			}
		}();

		ss << ' ' << c << " |";
	}

	ss << '\n' << Delimiter;

	return ss.str();
}
