#include "Evaluation.h"

#include "../Stats.h"
#include "../data/Board.h"
#include "../data/Psqt.h"
#include "../data/Pos.h"
#include "PieceAttacks.h"

#define S Score

constexpr short TEMPO_BONUS = 20;
constexpr S CHECK_BONUS(30, 42);

/*
 * These values were imported from the Stockfish Chess Engine
 */
constexpr S MINOR_THREATS[] =
{
	S(0, 0), S(6, 32), S(59, 41), S(79, 56), S(90, 119), S(79, 161), S(0, 0)
};
constexpr S OVERLOAD(12, 6);
constexpr S PAWN_DOUBLED(11, 56);
constexpr S PAWN_ISOLATED(5, 15);
constexpr short PAWN_CONNECTED[] =
{
	0, 7, 8, 12, 29, 48, 86
};
constexpr S PASSED_PAWN_RANK[] =
{
	S(0, 0), S(10, 28), S(17, 33), S(15, 41), S(62, 72), S(168, 177), S(276, 260)
};
constexpr S ROOK_ON_QUEEN_FILE(7, 6);
constexpr S ROOK_ON_PAWN(10, 28);
constexpr S ROOK_ON_FILE[] =
{
	S(21, 4), S(47, 25)
};
constexpr S ROOK_THREATS[] =
{
	S(0, 0), S(3, 44), S(38, 71), S(38, 61), S(0, 38), S(51, 38), S(0, 0)
};

constexpr S KNIGHT_MOBILITY[] =
{
    S(-62, -81), S(-53, -56), S(-12, -30), S(-4, -14), S(3, 8), S(13, 15),
    S(22, 23), S(28, 27), S(33, 33)
};
constexpr S BISHOP_MOBILITY[] =
{
	S(-48,-59), S(-20,-23), S(16, -3), S(26, 13), S(38, 24), S(51, 42),
	S(55, 54), S(63, 57), S(63, 65), S(68, 73), S(81, 78), S(81, 86),
	S(91, 88), S(98, 97)
};
constexpr S ROOK_MOBILITY[] =
{
    S(-58, -76), S(-27, -18), S(-15, 28), S(-10, 55), S(-5, 69), S(-2, 82),
    S(9, 112), S(16, 118), S(30, 132), S(29, 142), S(32, 155), S(38, 165),
    S(46, 166), S(48, 169), S(58, 171)
};
constexpr S QUEEN_MOBILITY[] =
{
    S(-39, -36), S(-21, -15), S(3, 8), S(3, 18), S(14, 34), S(22, 54),
    S(28, 61), S(41, 73), S(43, 79), S(48, 92), S(56, 94), S(60, 104),
    S(60, 113), S(66, 120), S(67, 123), S(70, 126), S(71, 133), S(73, 136),
    S(79, 140), S(88, 143), S(88, 148), S(99, 166), S(102, 170), S(102, 175),
    S(106, 184), S(109, 191), S(113, 206), S(116, 212)
};

#undef S

short Evaluation::evaluate(const Board &board) noexcept
{
	Stats::incrementBoardsEvaluated();

	Score totalScore[2]{};
	byte pawnCount[2]{};
	byte bishopCount[2]{};
	short npm[2]{};

	// 2 Bishops receive a bonus
	if (bishopCount[WHITE] >= 2)
		totalScore[WHITE] += 10;
	if (bishopCount[BLACK] >= 2)
		totalScore[BLACK] += 10;

	for (Color color = BLACK; color <= WHITE; color = Color(color + 1)) 
	{ 
		for (byte pieceType = PAWN; pieceType <= KING; ++pieceType) 
		{ 
			const Piece piece{ PieceType(pieceType), Color(color) }; 
			for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber) 
			{
				const byte square = board.pieceList[piece][pieceNumber];

				// Piece Evaluation
				Score piecePoints;
				switch (piece.type())
				{
				case PAWN:
					piecePoints = evaluatePawn(piece, square, board); break;
				case KNIGHT:
					piecePoints = evaluateKnight(piece, square, board); break;
				case BISHOP:
					piecePoints = evaluateBishop(piece, square, board); break;
				case ROOK:
					piecePoints = evaluateRook(piece, square, board); break;
				case QUEEN:
					piecePoints = evaluateQueen(piece, square, board); break;
				case KING:
					piecePoints = evaluateKing(piece, square, board); break;
				case NO_PIECE_TYPE: assert(false);
				}

				totalScore[piece.color()] += piecePoints;

				if (piece.type() == PAWN)
				{
					pawnCount[color]++;
					continue;
				}

				const int defendedValue = board.attackCount(color, square);
				const int attackedValue = board.attackCount(~color, square);
				Score points;

				if (defendedValue < attackedValue)
					points -= OVERLOAD * (attackedValue - defendedValue);

				if (piece.type() == BISHOP)
					bishopCount[color]++;
				
				if (board.isAttacked<KNIGHT>(~color, square))
					points -= MINOR_THREATS[piece.type()];
				if (board.isAttacked<BISHOP>(~color, square))
					points -= MINOR_THREATS[piece.type()];
				if (board.isAttacked<ROOK>(~color, square))
					points -= ROOK_THREATS[piece.type()];

				npm[color] += getPieceValue(piece.type());
				totalScore[color] += points;
			}
		} 
	} 

	if (board.isInCheck(~board.colorToMove))
		totalScore[board.colorToMove] += CHECK_BONUS;

	totalScore[board.colorToMove] += TEMPO_BONUS;

	const Score finalScore = totalScore[WHITE] - totalScore[BLACK];
	const Phase phase = board.getPhase();
	return phase == Phase::MIDDLE ? finalScore.mg : finalScore.eg;
}

Score Evaluation::evaluatePawn(const Piece &piece, const byte square, const Board &board) noexcept
{
	using namespace Bitboard;

	const Color color = piece.color();
	const Color oppositeColor = ~color;
	const Pos pos(square);
	Score value = Psqt::s_Bonus[PAWN][square];

	const Dir forwardDir = color ? NORTH : SOUTH;
	const byte rank = (color ? pos.y : 7u - pos.y) - 1u;
	const byte behind = color ? -1 : 1;

	const U64 adjacentFiles = getAdjacentFiles(square);
	const U64 opposed = board.getType(~piece) & getRay(forwardDir, square);
	const U64 neighbours = board.getType(piece) & adjacentFiles;
	const U64 connected = neighbours & getRank(square);
	const U64 support = neighbours & getRank(toSquare(pos.x + behind, pos.y));
	const bool isDoubled = board.at(pos.x, pos.y - behind) == piece;

	if (support | connected)
	{
		const int connectedScore = PAWN_CONNECTED[rank] * (2 + bool(connected) - bool(opposed))
			+ 21 * Bitboard::popCount(support);

		value += Score(connectedScore, connectedScore * (rank - 2) / 4);
	} else if (!neighbours)
		value -= PAWN_ISOLATED;

	if (!support && isDoubled)
		value -= PAWN_DOUBLED;

	{ // Passed Pawn
		U64 rays = getRay(forwardDir, square);
		const U64 adjacentRays = shift<WEST>(rays) | shift<EAST>(rays);
		rays |= color ? shift<NORTH>(adjacentRays) : shift<SOUTH>(adjacentRays);

		const bool isPassedPawn = !static_cast<bool>(rays & board.getType(oppositeColor, PAWN));

		if (isPassedPawn)
			value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

inline Score Evaluation::evaluateKnight(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Color color = piece.color();
	Score value = Psqt::s_Bonus[KNIGHT][square];

	const int mobility = Bitboard::popCount(PieceAttacks::getKnightAttacks(square) & ~board.allPieces[color]);
	value += KNIGHT_MOBILITY[mobility];

	return value;
}

Score Evaluation::evaluateBishop(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Color color = piece.color();
	Score value = Psqt::s_Bonus[BISHOP][square];

	const int mobility = Bitboard::popCount(PieceAttacks::getBishopAttacks(square, board.occupied) & ~board.allPieces[color]);
	value += BISHOP_MOBILITY[mobility];

	constexpr U64 centerFiles = FILE_D | FILE_E;
	constexpr U64 center = centerFiles & (RANK_4 | RANK_5);

	// Long Diagonal Bishop
	const U64 centerAttacks = PieceAttacks::getBishopAttacks(square, board.getType(~piece.color(), PAWN)) & center;
	if (Bitboard::popCount(centerAttacks) > 1)
		value.mg += 45;

	return value;
}

Score Evaluation::evaluateRook(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Color color = piece.color();
	const Color oppositeColor = ~color;
	const Pos pos(square);
	Score value = Psqt::s_Bonus[ROOK][pos.toSquare()];

	const int mobility = Bitboard::popCount(PieceAttacks::getRookAttacks(square, board.occupied) & ~board.allPieces[color]);
	value += ROOK_MOBILITY[mobility];

	const int rookOnPawn = [&] {
		if ((color && pos.y < 5) || (!color && pos.y > 4)) return 0;

		const U64 rays = Bitboard::getRank(square) | Bitboard::getFile(square);

		return Bitboard::popCount(rays & board.getType(~color, PAWN));
	}();
	value += ROOK_ON_PAWN * rookOnPawn;

	const auto rookOnFile = [&] {
		const U64 file = Bitboard::getFile(square);
		if (file & board.pieces[color][PAWN])
			return 0;
		if (file & board.pieces[oppositeColor][PAWN])
			return 1;
		return 2;
	}();
	value += ROOK_ON_FILE[rookOnFile];

	const U64 queens = board.getType(color, QUEEN) | board.getType(oppositeColor, QUEEN);
	if (Bitboard::getFile(square) & queens)
		value += ROOK_ON_QUEEN_FILE;

	return value;
}

Score Evaluation::evaluateQueen(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Color color = piece.color();
	const Pos pos(square);
	Score value = Psqt::s_Bonus[QUEEN][square];

	const int mobility = Bitboard::popCount(PieceAttacks::getQueenAttacks(square, board.occupied) & ~board.allPieces[color]);
	value += QUEEN_MOBILITY[mobility];

	if (const U64 initialPosition = FILE_D & (color ? RANK_1 : RANK_8);
		(initialPosition & Bitboard::shiftedBoards[square]) == 0)
		value.mg -= 35;

	return value;
}

inline Score Evaluation::evaluateKing(const Piece &piece, const byte square, const Board &board) noexcept
{
	Score value = Psqt::s_Bonus[KNIGHT][square];
	const Color color = piece.color();

	if (board.isCastled(color))
		value.mg += 57;
	else {
		const short count = short(board.canCastleKs(color)) + short(board.canCastleQs(color));
		value.mg += count * 20;
	}

	return value;
}
