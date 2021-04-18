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
	Board temp = *this;
	const bool result = FenParser::parseFen(temp, fen);

	if (result)
		*this = temp;

	return result;
}

std::string Board::getFen() const
{
	return FenParser::exportToFen(*this);
}

bool Board::isDrawn() const noexcept
{
	// Fifty Move Rules
	if (state.fiftyMoveRule > 99)
		return true;

	// Three-fold repetition
	i32 repetitions = 1;
	for (i32 i = historyPly - state.fiftyMoveRule; i < historyPly - 1; ++i)
		if (state.zKey == history[i].zKey && ++repetitions == 3)
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
		   && (!((knight | bishop).several()) || (bishop.empty() && knight.count() <= 2));
}

Phase Board::getPhase() const noexcept
{
	constexpr short midGameLimit = 15258;
	constexpr short endGameLimit = 3915;

	const short limit = std::max(endGameLimit, std::min(npm, midGameLimit));
	return Phase(((limit - endGameLimit) * 128) / (midGameLimit - endGameLimit));
}

void Board::makeMove(const Move move) noexcept
{
	makeMove(move, doesMoveGiveCheck(move));
}

void Board::makeMove(const Move move, const bool moveGivesCheck) noexcept
{
	assert(!move.empty());
	const Square from = move.from();
	const Square to = move.to();
	const Color side = colorToMove;
	const auto flags = move.flags();
	const PieceType movedPiece = move.piece();

	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);
	assert(Piece::isValid(movedPiece));
	assert(getPiece(from).isValid());
	assert(getPiece(from).type() == movedPiece);
	assert(move.capturedPiece() != KING);

	// Store the position info in history
	history[historyPly] = state;
	history[historyPly].moveContents = move.getContents();

	// Handle en passant capture and castling
	auto &castlingRights = state.castlingRights;
	auto &enPassantSq = state.enPassantSq;

	if (flags.enPassant())
	{
		assert(to == enPassantSq);
		removePiece(capturedEnPassantSq(side, enPassantSq));
	} else if (flags.kSideCastle())
	{
		const Square rookFrom = shiftToKingRank(colorToMove, SQ_H1);
		const Square rookTo = shiftToKingRank(colorToMove, SQ_F1);
		movePiece(rookFrom, rookTo);

		castlingRights |= (side ? CASTLED_WHITE : CASTLED_BLACK);
	} else if (flags.qSideCastle())
	{
		const Square rookFrom = shiftToKingRank(colorToMove, SQ_A1);
		const Square rookTo = shiftToKingRank(colorToMove, SQ_D1);
		movePiece(rookFrom, rookTo);

		castlingRights |= (side ? CASTLED_WHITE : CASTLED_BLACK);
	}

	Zobrist::xorEnPassant(state.zKey, enPassantSq);
	Zobrist::xorCastlingRights(state.zKey, CastlingRights(castlingRights));

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
	++state.fiftyMoveRule;

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
		state.fiftyMoveRule = 0;
	}

	++historyPly;
	++ply;

	if (movedPiece == PAWN)
	{
		state.fiftyMoveRule = 0;

		if (move.flags().doublePawnPush())
		{
			enPassantSq = capturedEnPassantSq(~side, from);

			Zobrist::xorEnPassant(state.zKey, enPassantSq);
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
	Zobrist::flipSide(state.zKey);

	updateNonPieceBitboards();

	assert((generateAttackers(getKingSq(~colorToMove)) & getPieces(colorToMove)).empty());

	state.kingAttackers = {};
	if (moveGivesCheck)
		state.kingAttackers = generateAttackers(getKingSq(colorToMove)) & getPieces(~colorToMove);

	computeCheckInfo();
}

void Board::undoMove() noexcept
{
	--historyPly;
	--ply;

	const BoardState &previousState = history[historyPly];
	const Move move = previousState.getMove();
	const Square from = move.from();
	const Square to = move.to();
	const auto flags = move.flags();

	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);

	state = previousState;

	colorToMove = ~colorToMove;

	if (flags.enPassant())
	{
		const Square capturedSq = capturedEnPassantSq(colorToMove, to);
		addPiece(capturedSq, { PAWN, ~colorToMove });
	} else if (flags.kSideCastle())
	{
		const Square rookTo = shiftToKingRank(colorToMove, SQ_F1);
		const Square rookFrom = shiftToKingRank(colorToMove, SQ_H1);
		movePiece(rookTo, rookFrom);
	} else if (flags.qSideCastle())
	{
		const Square rookTo = shiftToKingRank(colorToMove, SQ_D1);
		const Square rookFrom = shiftToKingRank(colorToMove, SQ_A1);
		movePiece(rookTo, rookFrom);
	}

	movePiece(to, from);

	if (const PieceType capturedType = move.capturedPiece();
		capturedType != NO_PIECE_TYPE)
		addPiece(to, { capturedType, ~colorToMove });

	if (flags.promotion())
	{
		removePiece(from);
		addPiece(from, { PAWN, colorToMove });
	}

	state.zKey = previousState.zKey;
	updateNonPieceBitboards();
}

void Board::makeNullMove() noexcept
{
	assert(!isSideInCheck());

	history[historyPly] = state;
	history[historyPly].moveContents = {};
	++historyPly;
	++ply;

	Zobrist::xorEnPassant(state.zKey, state.enPassantSq);
	state.enPassantSq = SQ_NONE;

	colorToMove = ~colorToMove;
	Zobrist::flipSide(state.zKey);

	computeCheckInfo();
}

void Board::undoNullMove() noexcept
{
	--historyPly;
	--ply;

	state = history[historyPly];

	colorToMove = ~colorToMove;
}

bool Board::doesMoveGiveCheck(const Move move) const noexcept
{
	const Square from = move.from();
	const Square to = move.to();

	auto &checkSq = state.possibleCheckSquares[move.piece()];
	if ((checkSq & Bitboard::fromSquare(to)).notEmpty())
		return true;

	const Square enemyKingSq = getKingSq(~colorToMove);

	if ((getKingBlockers(~colorToMove) & Bitboard::fromSquare(from)).notEmpty()
		&& !Bitboard::areAligned(enemyKingSq, from, to))
		return true;

	const auto flags = move.flags();

	if (flags.promotion())
	{
		return (Attacks::pieceAttacks(move.promotedPiece(), to, getPieces() ^ Bitboard::fromSquare(from))
				& Bitboard::fromSquare(enemyKingSq)).notEmpty();
	}

	if (flags.kSideCastle() | flags.qSideCastle())
	{
		Square rookFrom, rookTo;

		if (flags.kSideCastle())
		{
			rookTo = shiftToKingRank(colorToMove, SQ_F1);
			rookFrom = shiftToKingRank(colorToMove, SQ_H1);
		} else // Must be a Queen Side Castle
		{
			rookFrom = shiftToKingRank(colorToMove, SQ_A1);
			rookTo = shiftToKingRank(colorToMove, SQ_D1);
		}

		const auto blockers = (getPieces() ^ Bitboard::fromSquare(rookFrom) ^ Bitboard::fromSquare(from))
							  | Bitboard::fromSquare(rookTo) | Bitboard::fromSquare(to);
		return (Attacks::rookXRayAttacks(rookTo) & Bitboard::fromSquare(enemyKingSq)).notEmpty()
			   && (Attacks::rookAttacks(rookTo, blockers) & Bitboard::fromSquare(enemyKingSq)).notEmpty();
	}

	if (flags.enPassant())
	{
		const Square captured = capturedEnPassantSq(colorToMove, getEnPassantSq());
		const auto occupancy = (getPieces() ^ Bitboard::fromSquare(from) ^ Bitboard::fromSquare(captured))
							   | Bitboard::fromSquare(to);

		const auto queens = getPieces(QUEEN, colorToMove);
		return (
			(Attacks::rookAttacks(enemyKingSq, occupancy) & (getPieces(ROOK, colorToMove) | queens)) |
			(Attacks::bishopAttacks(enemyKingSq, occupancy) & (getPieces(BISHOP, colorToMove) | queens))
		).notEmpty();
	}

	return false;
}

bool Board::isMoveLegal(const Move move) const noexcept
{
	assert(!move.empty());
	const auto from = move.from();
	const auto to = move.to();
	const Square kingSq = getKingSq(colorToMove);

	assert(from < SQUARE_NB);
	assert(to < SQUARE_NB);
	assert(Piece::isValid(move.piece()));
	assert(getPiece(from).isValid());
	assert(getPiece(from).type() == move.piece());
	assert(getPiece(to).type() == move.capturedPiece());

	if (move.flags().enPassant())
	{
		const Square capturedSquare = capturedEnPassantSq(colorToMove, getEnPassantSq());

		const auto newOccupied = (getPieces() & ~(Bitboard::fromSquare(from) | Bitboard::fromSquare(capturedSquare)))
								 | Bitboard::fromSquare(to);

		assert(move.piece() == PAWN);
		assert(getPiece(move.from()).type() == PAWN);
		assert(getPiece(capturedSquare).type() == PAWN);

		const auto bishops = getPieces(QUEEN, ~colorToMove) | getPieces(BISHOP, ~colorToMove);
		const auto rooks = getPieces(QUEEN, ~colorToMove) | getPieces(ROOK, ~colorToMove);

		return (Attacks::bishopAttacks(kingSq, newOccupied) & bishops).empty() &&
			   (Attacks::rookAttacks(kingSq, newOccupied) & rooks).empty();
	}

	// TODO: Move castling checking here

	if (move.piece() == KING)
		return generateAttackers(~colorToMove, move.to(), getPieces() ^ Bitboard::fromSquare(from)).empty();

	return (getKingBlockers(colorToMove) & Bitboard::fromSquare(from)).empty()
		   || Bitboard::areAligned(from, kingSq, to);
}

Bitboard Board::generateAttackers(Color attackerColor, Square sq, Bitboard blockers) const noexcept
{
	return generateAttackers(sq, blockers) & getPieces(attackerColor);
}

Bitboard Board::generateAttackers(Square sq, Bitboard blockers) const noexcept
{
	const auto queens = getPieces(QUEEN);
	const auto bishops = getPieces(BISHOP) | queens;
	const auto rooks = getPieces(ROOK) | queens;

	return (getPieces(PAWN, WHITE) & Attacks::pawnAttacks<BLACK>(Bitboard::fromSquare(sq)))
		   | (getPieces(PAWN, BLACK) & Attacks::pawnAttacks<WHITE>(Bitboard::fromSquare(sq)))
		   | (getPieces(KNIGHT) & Attacks::knightAttacks(sq))
		   | (getPieces(KING) & Attacks::kingAttacks(sq))
		   | (bishops & Attacks::bishopAttacks(sq, blockers))
		   | (rooks & Attacks::rookAttacks(sq, blockers));
}

Bitboard Board::generateAttackers(Square sq) const noexcept
{
	return generateAttackers(sq, getPieces());
}

bool Board::isSideInCheck() const noexcept
{
	return state.kingAttackers.notEmpty();
}

void Board::addPiece(const Square square, const Piece piece) noexcept
{
	assert(piece.isValid());

	Zobrist::xorPiece(state.zKey, square, piece);
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

	Zobrist::xorPiece(state.zKey, from, piece);
	getPiece(from) = EmptyPiece;
	getPieces(piece).removeSquare(from);

	assert(!getPiece(to).isValid());
	Zobrist::xorPiece(state.zKey, to, piece);
	getPiece(to) = piece;
	getPieces(piece).addSquare(to);

	[[maybe_unused]] bool pieceMoved{};
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

	Zobrist::xorPiece(state.zKey, square, piece);
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

Bitboard Board::findBlockers(const Bitboard sliders, const Square sq, Bitboard &pinners) const noexcept
{
	Bitboard blockers{};
	pinners = {};

	const auto rooks = Attacks::rookXRayAttacks(sq) & (getPieces(ROOK) | getPieces(QUEEN));
	const auto bishops = Attacks::bishopXRayAttacks(sq) & (getPieces(BISHOP) | getPieces(QUEEN));
	Bitboard hiddenPinners = (rooks | bishops) & sliders;
	const Bitboard occupancy = getPieces() & ~hiddenPinners;

	while (hiddenPinners.notEmpty())
	{
		const Square hiddenSq = hiddenPinners.popLsb();
		const auto bb = Bitboard::fromBetween(sq, hiddenSq) & occupancy;

		if (bb.notEmpty() && !bb.several())
		{
			blockers |= bb;
			if ((getPieces(getPiece(sq).color()) & bb).notEmpty())
				pinners |= Bitboard::fromSquare(hiddenSq);
		}
	}

	return blockers;
}

void Board::computeCheckInfo() noexcept
{
	state.kingBlockers[WHITE] = findBlockers(getPieces(BLACK), getKingSq(WHITE), state.kingPinners[BLACK]);
	state.kingBlockers[BLACK] = findBlockers(getPieces(WHITE), getKingSq(BLACK), state.kingPinners[WHITE]);

	const Square enemyKingSq = getKingSq(~colorToMove);

	auto &checkSquares = state.possibleCheckSquares;
	checkSquares[PAWN] = Attacks::pawnAttacks(~colorToMove, Bitboard::fromSquare(enemyKingSq));
	checkSquares[KNIGHT] = Attacks::knightAttacks(enemyKingSq);
	checkSquares[BISHOP] = Attacks::bishopAttacks(enemyKingSq, getPieces());
	checkSquares[ROOK] = Attacks::rookAttacks(enemyKingSq, getPieces());
	checkSquares[QUEEN] = checkSquares[BISHOP] | checkSquares[ROOK];
	checkSquares[KING] = {};
}

void Board::updatePieceList() noexcept
{
	pieceCount.fill({});

	for (Square sq{}; sq < SQUARE_NB; ++sq)
	{
		const Piece piece = getPiece(sq);
		if (piece)
		{
			pieceList[piece][pieceCount[piece]++] = sq;

			getPieces(piece).addSquare(sq);

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

	static constexpr auto Delimiter = "\n+---+---+---+---+---+---+---+---+\n";

	for (Square sq{}; sq < SQUARE_NB; ++sq)
	{
		if (fileOf(sq) == 0)
			ss << Delimiter << '|';

		const Piece piece = getPiece(sq);
		const auto type = piece.type();
		const bool color = piece.color() == WHITE;

		const char c = [&]
		{
			switch (type)
			{
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
				default:
					return ' ';
			}
		}();

		ss << ' ' << c << " |";
	}

	ss << Delimiter;

	ss << "\nFEN: " << getFen() << '\n';

	return ss.str();
}
