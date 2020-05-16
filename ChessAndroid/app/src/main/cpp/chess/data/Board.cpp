#include "Board.h"

#include "../algorithm/Hash.h"
#include "../algorithm/Evaluation.h"
#include "../algorithm/MoveGen.h"
#include "../persistence/FenParser.h"

void Board::initDefaultBoard()
{
	setToFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool Board::setToFen(const std::string &fen)
{
	FenParser parser(*this);
	return parser.parseFen(fen);
}

bool Board::canCastle(const Color color) const noexcept
{
	return color == BLACK ? canCastle<BLACK>() : canCastle<WHITE>();
}

Piece &Board::getPiece(const byte squareIndex) noexcept
{
	return data[squareIndex];
}

Piece Board::getPiece(const byte squareIndex) const noexcept
{
	return data[squareIndex];
}

U64 &Board::getType(const Piece piece) noexcept
{
	return pieces[piece.color()][piece.type()];
}

U64 Board::getType(const Piece piece) const noexcept
{
	return pieces[piece.color()][piece.type()];
}

U64 &Board::getType(const PieceType type, const Color color) noexcept
{
	return pieces[color][type];
}

U64 Board::getType(const PieceType type, const Color color) const noexcept
{
	return pieces[color][type];
}

bool Board::isDrawn() const noexcept
{
	// Fifty Move Rules
	if (fiftyMoveRule > 99)
		return true;

	// Three-fold repetition
	byte repetitions = 1;
	for (byte i = historyPly - fiftyMoveRule; i < historyPly - 1; ++i)
	{
		if (zKey == history[i].zKey)
		{
			++repetitions;
			if (repetitions == 3)
				return true;
		}
	}

	// Insufficient Material
	// KvK, KvN, KvB, KvNN
	const U64 pawns = pieces[BLACK][PAWN] | pieces[WHITE][PAWN];
	const U64 knight = pieces[BLACK][KNIGHT] | pieces[WHITE][KNIGHT];
	const U64 bishop = pieces[BLACK][BISHOP] | pieces[WHITE][BISHOP];
	const U64 rooks = pieces[BLACK][ROOK] | pieces[WHITE][ROOK];
	const U64 queens = pieces[BLACK][QUEEN] | pieces[WHITE][QUEEN];

	return !(pawns | rooks | queens)
		   && (!Bits::several(allPieces[WHITE]) || !Bits::several(allPieces[BLACK]))
		   && (!Bits::several(knight | bishop) || (!bishop && Bits::popCount(knight) <= 2));
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
	const byte from = move.from();
	const byte to = move.to();
	const Color side = colorToMove;
	const auto flags = move.flags();
	const U64 posKey = zKey;
	const PieceType movedPiece = move.piece();

	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);
	assert(Piece::isValid(movedPiece));
	{
		const auto p = getPiece(from);
		assert(p.isValid());
	}

	assert(getPiece(from).type() == movedPiece);

	// Handle en passant capture and castling
	if (flags & Move::EN_PASSANT)
	{
		const byte capturedSq = to + static_cast<byte>(side ? -8 : 8);
		removePiece(capturedSq);
	} else if (flags & Move::KSIDE_CASTLE)
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
	} else if (flags & Move::QSIDE_CASTLE)
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

	Hash::xorEnPassant(zKey, enPassantSq);
	Hash::xorCastlingRights(zKey, CastlingRights(castlingRights));

	// Store the position info in history
	history[historyPly] = { posKey, kingAttackers, move.getContents(), castlingRights, enPassantSq,
							fiftyMoveRule };

	if (canCastle(side))
	{
		if (movedPiece == ROOK)
		{
			const U64 rookFile = Bits::getFile(from);
			if (rookFile == FILE_A)
				castlingRights &= ~(side ? CASTLE_WHITE_QUEEN : CASTLE_BLACK_QUEEN);
			else if (rookFile == FILE_H)
				castlingRights &= ~(side ? CASTLE_WHITE_KING : CASTLE_BLACK_KING);
		} else if (movedPiece == KING)
			// Remove all castling rights if the king is moved
			castlingRights &= ~(side ? CASTLE_WHITE_BOTH : CASTLE_BLACK_BOTH);
	}

	enPassantSq = SQ_NONE;
	++fiftyMoveRule;

	if (const PieceType capturedType = move.capturedPiece();
		capturedType != PieceType::NO_PIECE_TYPE)
	{
		assert(Piece::isValid(capturedType));
		if (capturedType == ROOK && canCastle(~side))
		{
			const U64 rookFile = Bits::getFile(to);
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

		if (move.flags() & Move::DOUBLE_PAWN_PUSH)
		{
			enPassantSq = from + static_cast<byte>(side ? 8 : -8);

			Hash::xorEnPassant(zKey, enPassantSq);
			const U64 enPassantRank = Bits::getRank(enPassantSq);
			assert(enPassantRank == RANK_3 || enPassantRank == RANK_6);
		}
	}

	movePiece(from, to);

	if (move.flags() & Move::PROMOTION)
	{
		const PieceType promotedPiece = move.promotedPiece();
		assert(Piece::isValid(promotedPiece));
		assert(promotedPiece != PAWN && promotedPiece != KING);

		removePiece(to); // Remove Pawn
		addPiece(to, { promotedPiece, side });
	}

	colorToMove = ~colorToMove;
	Hash::flipSide(zKey);

	updateNonPieceBitboards();

	if ((flags & Move::CAPTURE && move.capturedPiece() == KING)
		|| (side ? allKingAttackers<WHITE>() : allKingAttackers<BLACK>()))
	{
		undoMove();
		return false;
	}

	kingAttackers = colorToMove ? allKingAttackers<WHITE>() : allKingAttackers<BLACK>();

	return true;
}

void Board::undoMove() noexcept
{
	--historyPly;
	--ply;

	const UndoMove &previousMove = history[historyPly];
	const Move move = previousMove.getMove();
	const byte from = move.from();
	const byte to = move.to();
	const auto flags = move.flags();

	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);

	kingAttackers = previousMove.kingAttackers;
	castlingRights = previousMove.castlingRights;
	fiftyMoveRule = previousMove.fiftyMoveRule;
	enPassantSq = previousMove.enPassantSq;

	colorToMove = ~colorToMove;

	if (flags & Move::EN_PASSANT)
	{
		const byte capturesSq = to + static_cast<byte>(colorToMove ? -8 : 8);
		addPiece(capturesSq, { PAWN, ~colorToMove });
	} else if (flags & Move::KSIDE_CASTLE)
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
	} else if (flags & Move::QSIDE_CASTLE)
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

	if (flags & Move::PROMOTION)
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

	Hash::xorEnPassant(zKey, enPassantSq);
	enPassantSq = SQ_NONE;

	colorToMove = ~colorToMove;
	Hash::flipSide(zKey);

	kingAttackers = colorToMove ? allKingAttackers<WHITE>() : allKingAttackers<BLACK>();
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

bool Board::isAttackedByAny(const Color attackerColor, const byte targetSquare) const noexcept
{
	return isAttacked<PAWN>(attackerColor, targetSquare)
		   || isAttacked<KNIGHT>(attackerColor, targetSquare)
		   || isAttacked<KING>(attackerColor, targetSquare)
		   || isAttacked<BISHOP>(attackerColor, targetSquare)
		   || isAttacked<ROOK>(attackerColor, targetSquare)
		   || isAttacked<QUEEN>(attackerColor, targetSquare);
}

bool Board::isSideInCheck() const noexcept
{
	return static_cast<bool>(kingAttackers);
}

void Board::addPiece(const byte square, const Piece piece) noexcept
{
	assert(piece.isValid());

	Hash::xorPiece(zKey, square, piece);
	getType(piece) |= Bits::getSquare64(square);
	getPiece(square) = piece;
	if (piece.type() != PAWN)
		npm += Evaluation::getPieceValue(piece.type());

	pieceList[piece][pieceCount[piece]++] = square;
}

void Board::movePiece(const byte from, const byte to) noexcept
{
	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);

	const Piece piece = getPiece(from);
	assert(piece.isValid());

	Hash::xorPiece(zKey, from, piece);
	getPiece(from) = Piece();
	getType(piece) &= ~Bits::getSquare64(from);

	Hash::xorPiece(zKey, to, piece);
	getPiece(to) = piece;
	getType(piece) |= Bits::getSquare64(to);

	bool pieceMoved = false;
	for (byte &sq : pieceList[piece])
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

void Board::removePiece(const byte square) noexcept
{
	assert(square < SQUARE_NB);

	const Piece piece = getPiece(square);
	assert(piece.isValid());

	Hash::xorPiece(zKey, square, piece);
	getType(piece) &= ~Bits::getSquare64(square);
	getPiece(square) = Piece();

	if (piece.type() != PAWN)
		npm -= Evaluation::getPieceValue(piece.type());

	auto &piecesSquares = pieceList[piece];

	byte pieceIndex = -1;
	for (byte i = 0; i < pieceCount[piece]; ++i)
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
	for (byte sq{}; sq < SQUARE_NB; ++sq)
	{
		const Piece piece = data[sq];
		const U64 bb = Bits::getSquare64(sq);

		getType(piece) |= bb;

		if (piece.type() != PAWN)
			npm += Evaluation::getPieceValue(piece.type());
	}

	pieceCount.fill({});

	for (byte square{}; square < SQUARE_NB; ++square)
		if (const Piece piece = getPiece(square))
			pieceList[piece][pieceCount[piece]++] = square;
}

void Board::updateNonPieceBitboards() noexcept
{
	allPieces[BLACK] = getType(PAWN, BLACK)
					   | getType(KNIGHT, BLACK)
					   | getType(BISHOP, BLACK)
					   | getType(ROOK, BLACK)
					   | getType(QUEEN, BLACK)
					   | getType(KING, BLACK);

	allPieces[WHITE] = getType(PAWN, WHITE)
					   | getType(KNIGHT, WHITE)
					   | getType(BISHOP, WHITE)
					   | getType(ROOK, WHITE)
					   | getType(QUEEN, WHITE)
					   | getType(KING, WHITE);

	occupied = allPieces[BLACK] | allPieces[WHITE];
}
