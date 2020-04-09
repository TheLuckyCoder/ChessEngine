#include "Evaluation.h"

#include "../Stats.h"
#include "../data/Board.h"
#include "../data/Psqt.h"
#include "../data/Pos.h"
#include "PieceAttacks.h"

#define S Score

namespace
{
	constexpr short TEMPO_BONUS = 20;
	constexpr S CHECK_BONUS{ 30, 42 };

	/*
	 * These values were imported from the Stockfish Chess Engine
	 */
	constexpr S PAWN_DOUBLED{ 11, 56 };
	constexpr S PAWN_ISOLATED{ 5, 15 };
	constexpr short PAWN_CONNECTED[]
	{
		0, 7, 8, 12, 29, 48, 86
	};
	constexpr S PASSED_PAWN_RANK[]
	{
		{ 0, 0 }, { 10, 28 }, { 17, 33 },  { 15, 41 }, {62, 72 }, { 168, 177 }, { 276, 260 }
	};

	constexpr S MINOR_THREATS[]
	{
		S(0, 0), S(6, 32), S(59, 41), S(79, 56), S(90, 119), S(79, 161), S(0, 0)
	};
	constexpr S MINOR_PAWN_SHIELD{ 18,  3 };

	constexpr S ROOK_ON_PAWN{ 10, 28 };
	constexpr S ROOK_ON_QUEEN_FILE{ 7, 6 };
	constexpr S TRAPPED_ROOK{ 52, 10 };
	constexpr S ROOK_ON_FILE[] =
	{
		{ 21, 4 }, { 47, 25 }
	};
	constexpr S ROOK_THREATS[]
	{
		{ 0, 0 }, { 3, 44 }, { 38, 71 }, { 38, 61 }, { 0, 38 }, { 51, 38 }, { 0, 0 }
	};

	constexpr S KNIGHT_MOBILITY[]
	{
		{ -62, -81 }, { -53, -56 }, { -12, -30 }, { -4, -14 }, { 3, 8 }, { 13, 15 },
		{ 22, 23 }, { 28, 27 }, { 33, 33 }
	};
	constexpr S BISHOP_MOBILITY[]
	{
		{ -48,-59 }, { -20,-23 }, { 16, -3 }, { 26, 13 }, { 38, 24 }, { 51, 42 },
		{ 55, 54 }, { 63, 57 }, { 63, 65 }, { 68, 73 }, { 81, 78 }, { 81, 86 },
		{ 91, 88 }, { 98, 97 }
	};
	constexpr S ROOK_MOBILITY[]
	{
		{ -58, -76 }, { -27, -18 }, { -15, 28 }, { -10, 55 }, { -5, 69 }, { -2, 82 },
		{ 9, 112 }, { 16, 118 }, { 30, 132 }, { 29, 142 }, { 32, 155 }, { 38, 165 },
		{ 46, 166 }, { 48, 169 }, { 58, 171 }
	};
	constexpr S QUEEN_MOBILITY[]
	{
		{ -39, -36 }, { -21, -15 }, { 3, 8 }, { 3, 18 }, { 14, 34 }, { 22, 54 },
		{ 28, 61 }, { 41, 73 }, { 43, 79 }, { 48, 92 }, { 56, 94 }, { 60, 104 },
		{ 60, 113 }, { 66, 120 }, { 67, 123 }, { 70, 126 }, { 71, 133 }, { 73, 136 },
		{ 79, 140 }, { 88, 143 }, { 88, 148 }, { 99, 166 }, { 102, 170 }, { 102, 175 },
		{ 106, 184 }, { 109, 191 }, { 113, 206 }, { 116, 212 }
	};
}

#undef S

using namespace Bits;

short Evaluation::evaluate(const Board &board) noexcept
{
	Stats::incrementBoardsEvaluated();

	Score totalScore[2]{};
	short npm[2]{};

	for (Color color = BLACK; color <= WHITE; color = Color(color + 1))
	{
		for (byte pieceType = KNIGHT; pieceType <= QUEEN; ++pieceType)
		{
			const Piece piece{ PieceType(pieceType), Color(color) };
			for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
			{
				const byte square = board.pieceList[piece][pieceNumber];

				Score points;

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

	const Score piecesScore = evaluatePieces<WHITE>(board) -  evaluatePieces<BLACK>(board);
	const Score finalScore = totalScore[WHITE] - totalScore[BLACK] + piecesScore;
	const Phase phase = board.getPhase();
	return phase == Phase::MIDDLE ? finalScore.mg : finalScore.eg;
}

template <Color Us>
Score Evaluation::evaluatePieces(const Board &board) noexcept
{
	Score pawnScore;

	{
		U64 pawns = board.getType(Us, PAWN);
		if constexpr (Us == BLACK)
			pawns = flipVertical(pawns);

		const auto pawnsEntry = _pawnTable[pawns];

		if (pawnsEntry.pawns != pawns)
		{
			constexpr Piece piece{ PAWN, Us };
			for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
				pawnScore += evaluatePawn<Us>(board, board.pieceList[piece][pieceNumber]);

			_pawnTable.insert({ pawns, pawnScore });
		} else
			pawnScore = pawnsEntry.score;
	}

	Score score;

	const auto knightBishop = [&] (const byte square)
	{
		constexpr Dir Behind = Us ? Dir::SOUTH : Dir::NORTH;

		if (Bits::shift<Behind>(board.getType(Us, PAWN)) & Bits::getSquare64(square))
			score += MINOR_PAWN_SHIELD;
	};

	Piece piece{ KNIGHT, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateKnight<Us>(board, square);
		knightBishop(square);
	}

	piece = { BISHOP, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateBishop<Us>(board, square);
		knightBishop(square);
	}

	if (board.pieceCount[piece] >= 2)
		score += 10;

	piece = { ROOK, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
		score += evaluateRook<Us>(board, board.pieceList[piece][pieceNumber]);

	piece = { QUEEN, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
		score += evaluateQueen<Us>(board, board.pieceList[piece][pieceNumber]);

	piece = { KING, Us };
	score += evaluateKing<Us>(board, board.pieceList[piece][0]);

	return score + pawnScore;
}

template <Color Us>
Score Evaluation::evaluatePawn(const Board &board, const byte square) noexcept
{
	constexpr Color Them = ~Us;
	constexpr Dir forwardDir = Us ? NORTH : SOUTH;
	constexpr byte behind = Us ? -1 : 1;
	constexpr Piece piece{ PAWN, Us };

	Score value = Psqt::BONUS[PAWN][square];

	const Pos pos(square);
	const byte rank = (Us ? pos.y : 7u - pos.y) - 1u;

	const U64 adjacentFiles = getAdjacentFiles(square);
	const U64 opposed = board.getType(Piece(PAWN, Them)) & getRay(forwardDir, square);
	const U64 neighbours = board.getType(piece) & adjacentFiles;
	const U64 connected = neighbours & getRank(square);
	const U64 support = neighbours & getRank(toSquare(pos.x + behind, pos.y));
	const bool isDoubled = board.at(pos.x, pos.y - behind) == piece;

	if (support | connected)
	{
		const int connectedScore = PAWN_CONNECTED[rank] * (2 + bool(connected) - bool(opposed))
								   + 21 * popCount(support);

		value += Score(connectedScore, connectedScore * (rank - 2) / 4);
	} else if (!neighbours)
		value -= PAWN_ISOLATED;

	if (!support && isDoubled)
		value -= PAWN_DOUBLED;

	{ // Passed Pawn
		U64 rays = getRay(forwardDir, square);
		const U64 adjacentRays = shift<WEST>(rays) | shift<EAST>(rays);
		rays |= Us ? shift<NORTH>(adjacentRays) : shift<SOUTH>(adjacentRays);

		const bool isPassedPawn = !bool(rays & board.getType(Them, PAWN));

		if (isPassedPawn)
			value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

template <Color Us>
Score Evaluation::evaluateKnight(const Board &board, const byte square) noexcept
{
	Score value = Psqt::BONUS[KNIGHT][square];

	const int mobility = popCount(PieceAttacks::getKnightAttacks(square) & ~board.allPieces[Us]);
	value += KNIGHT_MOBILITY[mobility];

	return value;
}

template <Color Us>
Score Evaluation::evaluateBishop(const Board &board, const byte square) noexcept
{
	constexpr Color Them = ~Us;
	constexpr U64 CenterFiles = FILE_D | FILE_E;
	constexpr U64 Center = CenterFiles & (RANK_4 | RANK_5);

	Score value = Psqt::BONUS[BISHOP][square];

	const int mobility = popCount(PieceAttacks::getBishopAttacks(square, board.occupied) & ~board.allPieces[Us]);
	value += BISHOP_MOBILITY[mobility];

	// Long Diagonal Bishop
	const U64 centerAttacks = PieceAttacks::getBishopAttacks(square, board.getType(Them, PAWN)) & Center;
	if (popCount(centerAttacks) > 1)
		value.mg += 45;

	return value;
}

template <Color Us>
Score Evaluation::evaluateRook(const Board &board, const byte square) noexcept
{
	constexpr Color Them = ~Us;

	const Pos pos(square);
	Score value = Psqt::BONUS[ROOK][pos.toSquare()];

	const int mobility = popCount(PieceAttacks::getRookAttacks(square, board.occupied) & ~board.allPieces[Us]);
	value += ROOK_MOBILITY[mobility];

	const int rookOnPawn = [&]
	{
		if ((Us && pos.y < 5) || (Them && pos.y > 4)) return 0;

		const U64 rays = getRank(square) | getFile(square);

		return popCount(rays & board.getType(Them, PAWN));
	}();
	value += ROOK_ON_PAWN * rookOnPawn;

	const U64 file = getFile(square);
	if (!(board.getType(Us, PAWN) & file))
		value += ROOK_ON_FILE[!(board.getType(Them, PAWN) & file)];
	else if (mobility <= 3)
	{
		const byte kx = col(board.pieceList[Piece{ KING, Us }][0]);

		if ((kx < 4) == (pos.x < kx))
			value -= TRAPPED_ROOK * (1 + !board.canCastle(Us));
	}

	const U64 queens = board.getType(Us, QUEEN) | board.getType(Them, QUEEN);
	if (file & queens)
		value += ROOK_ON_QUEEN_FILE;

	return value;
}

template <Color Us>
Score Evaluation::evaluateQueen(const Board &board, const byte square) noexcept
{
	Score value = Psqt::BONUS[QUEEN][square];

	const int mobility = popCount(PieceAttacks::getQueenAttacks(square, board.occupied) & ~board.allPieces[Us]);
	value += QUEEN_MOBILITY[mobility];

	if (const U64 initialPosition = FILE_D & (Us ? RANK_1 : RANK_8);
		(initialPosition & shiftedBoards[square]) == 0)
		value.mg -= 35;

	return value;
}

template <Color Us>
Score Evaluation::evaluateKing(const Board &board, const byte square) noexcept
{
	Score value = Psqt::BONUS[KNIGHT][square];

	if (board.isCastled(Us))
		value.mg += 57;
	else {
		const short count = short(board.canCastleKs(Us)) + short(board.canCastleQs(Us));
		value.mg += count * 19;
	}

	return value;
}
