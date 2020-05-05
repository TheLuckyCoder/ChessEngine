#include "Evaluation.h"

#include "../Stats.h"
#include "../data/Board.h"
#include "../data/Psqt.h"
#include "../data/Pos.h"
#include "PieceAttacks.h"

namespace
{
	constexpr short TEMPO_BONUS = 20;
	constexpr Score CHECK_BONUS{ 30, 42 };

	/*
	 * Most of these values were imported from the Stockfish Chess Engine
	 */
	constexpr Score PAWN_DOUBLED{ 11, 56 };
	constexpr Score PAWN_ISOLATED{ 5, 15 };
	constexpr short PAWN_CONNECTED[]
	{
		0, 7, 8, 12, 29, 48, 86
	};
	constexpr Score PASSED_PAWN_RANK[]
	{
		{ }, { 10, 28 }, { 17, 33 },  { 15, 41 }, {62, 72 }, { 168, 177 }, { 276, 260 }
	};

	constexpr Score MINOR_PAWN_SHIELD{ 18,  3 };
	constexpr Score MINOR_THREATS[]
	{
		{ }, { 6, 32 }, { 59, 41 }, { 79, 56 }, { 90, 119 }, { 79, 161 }, { }
	};
	constexpr Score KING_PROTECTOR{ 4, 5 };
	constexpr Score HANGING{ 69, 36 };
	constexpr Score RESTRICTED_PIECE_MOVEMENT{ 7,  7 };
	constexpr Score WEAK_QUEEN_PROTECTION{ 14,  0 };
	constexpr Score THREAT_BY_KING{ 24, 89 };
	constexpr Score THREAT_BY_SAFE_PAWN{ 173, 94 };
	constexpr Score QUEEN_KNIGHT_THREAT{ 16, 11 };
	constexpr Score QUEEN_SLIDER_THREAT{ 59, 18 };

	constexpr Score ROOK_ON_QUEEN_FILE{ 7, 6 };
	constexpr Score TRAPPED_ROOK{ 52, 10 };
	constexpr Score ROOK_ON_FILE[] =
	{
		{ 21, 4 }, { 47, 25 }
	};
	constexpr Score ROOK_THREATS[]
	{
		{ }, { 3, 44 }, { 38, 71 }, { 38, 61 }, { 0, 38 }, { 51, 38 }, { }
	};
	constexpr Score KING_PAWN_SHIELD{ 10, 0 };

	constexpr Score KNIGHT_MOBILITY[]
	{
		{ -62, -81 }, { -53, -56 }, { -12, -30 }, { -4, -14 }, { 3, 8 }, { 13, 15 },
		{ 22, 23 }, { 28, 27 }, { 33, 33 }
	};
	constexpr Score BISHOP_MOBILITY[]
	{
		{ -48,-59 }, { -20,-23 }, { 16, -3 }, { 26, 13 }, { 38, 24 }, { 51, 42 },
		{ 55, 54 }, { 63, 57 }, { 63, 65 }, { 68, 73 }, { 81, 78 }, { 81, 86 },
		{ 91, 88 }, { 98, 97 }
	};
	constexpr Score ROOK_MOBILITY[]
	{
		{ -58, -76 }, { -27, -18 }, { -15, 28 }, { -10, 55 }, { -5, 69 }, { -2, 82 },
		{ 9, 112 }, { 16, 118 }, { 30, 132 }, { 29, 142 }, { 32, 155 }, { 38, 165 },
		{ 46, 166 }, { 48, 169 }, { 58, 171 }
	};
	constexpr Score QUEEN_MOBILITY[]
	{
		{ -39, -36 }, { -21, -15 }, { 3, 8 }, { 3, 18 }, { 14, 34 }, { 22, 54 },
		{ 28, 61 }, { 41, 73 }, { 43, 79 }, { 48, 92 }, { 56, 94 }, { 60, 104 },
		{ 60, 113 }, { 66, 120 }, { 67, 123 }, { 70, 126 }, { 71, 133 }, { 73, 136 },
		{ 79, 140 }, { 88, 143 }, { 88, 148 }, { 99, 166 }, { 102, 170 }, { 102, 175 },
		{ 106, 184 }, { 109, 191 }, { 113, 206 }, { 116, 212 }
	};
}

PawnStructureTable Evaluation::_pawnTable{ 4 };

using namespace Bits;

namespace Masks
{
	constexpr auto PAWN_SHIELD = []
	{
		std::array<std::array<U64, SQUARE_NB>, 2> array{};

		for (byte i{}; i < SQUARE_NB; ++i)
		{
			const U64 square = getSquare64(i);

			array[WHITE][i] =
				(square << 8 | (square << 7 & ~FILE_H) | (square << 9 & ~FILE_A)) & RANK_2;
			array[BLACK][i] =
				(square >> 8 | (square >> 7 & ~FILE_A) | (square >> 9 & ~FILE_H)) & RANK_7;
		}

		return array;
	}();
}

Evaluation::Result Evaluation::evaluate(const Board &board) noexcept
{
	Result result{ board };
	Evaluation evaluator(board);

	Stats::incBoardsEvaluated();

	const Score piecesScore = evaluator.evaluatePieces<WHITE>() - evaluator.evaluatePieces<BLACK>();
	const Score attacksScore =
		evaluator.evaluateAttacks<WHITE>() - evaluator.evaluateAttacks<BLACK>();
	Score finalScore = piecesScore + attacksScore;

	if (board.colorToMove)
	{
		finalScore += TEMPO_BONUS;

		if (Bits::getSquare64(board.getKingSq(BLACK)) & evaluator._attacksAll[WHITE])
		{
			finalScore += CHECK_BONUS;
			result.isInCheck = true;
		}
	} else
	{
		finalScore -= TEMPO_BONUS;

		if (Bits::getSquare64(board.getKingSq(WHITE)) & evaluator._attacksAll[BLACK])
		{
			finalScore -= CHECK_BONUS;
			result.isInCheck = true;
		}
	}

	const Phase phase = board.getPhase();
	result.value = (finalScore.mg * phase + (finalScore.eg * (128 - phase))) / 128;
	return result;
}

Evaluation::Evaluation(const Board &board)
	: board(board)
{
}

template <Color Us>
Score Evaluation::evaluatePieces() noexcept
{
	constexpr Color Them = ~Us;
	Score score;
	{
		const byte square = board.getKingSq(Us);
		score += evaluateKing<Us>(square);

		const U64 attacks = PieceAttacks::kingAttacks(square);
		_attacks[Us][KING] = attacks;
		_attacksAll[Us] = attacks;
	}

	Score pawnScore;

	{
		U64 pawns = board.getType(PAWN, Us);

		if constexpr (Us == BLACK)
			pawns = flipVertical(pawns);

		const auto pawnsEntry = _pawnTable[pawns];

		if (pawnsEntry.pawns != pawns)
		{
			constexpr Piece piece{ PAWN, Us };

			for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
			{
				const byte square = board.pieceList[piece][pieceNumber];
				pawnScore += evaluatePawn<Us>(square);
			}

			_pawnTable.insert({ pawns, pawnScore });
		} else
			pawnScore = pawnsEntry.score;
	}

	{
		const U64 &pawns = board.getType(PAWN, Us);
		const U64 pawnsAttacks = PieceAttacks::pawnAttacks<Us>(pawns);
		const U64 doublePawnsAttacks = PieceAttacks::pawnDoubleAttacks<Us>(pawns);

		_attacks[Us][PAWN] = pawnsAttacks;
		_attacksMultiple[Us] = doublePawnsAttacks | (doublePawnsAttacks & _attacksAll[Us]);
		_attacksAll[Us] |= pawnsAttacks;
	}

	_mobilityArea[Us] =
		~board.allPieces[Us] & ~PieceAttacks::pawnAttacks<Them>(board.getType(PAWN, Them));

	const auto knightBishop = [&](const byte square)
	{
		constexpr Dir Behind = Us ? Dir::SOUTH : Dir::NORTH;

		score -= KING_PROTECTOR * getDistance(square, board.getKingSq(Us));

		if (getSquare64(square) & shift<Behind>(board.getType(PAWN, Us)))
			score += MINOR_PAWN_SHIELD;
	};

	Piece piece{ KNIGHT, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateKnight<Us>(square);
		knightBishop(square);

		const U64 attacks = PieceAttacks::knightAttacks(square);
		_attacks[Us][KNIGHT] |= attacks;
		_attacksMultiple[Us] |= _attacksAll[Us] & attacks;
		_attacksAll[Us] |= attacks;
	}

	piece = { BISHOP, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateBishop<Us>(square);
		knightBishop(square);

		const U64 attacks = PieceAttacks::bishopAttacks(square, board.occupied);
		_attacks[Us][BISHOP] |= attacks;
		_attacksMultiple[Us] |= _attacksAll[Us] & attacks;
		_attacksAll[Us] |= attacks;
	}

	if (board.pieceCount[piece] >= 2)
		score += 40;

	piece = { ROOK, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateRook<Us>(square);

		const U64 attacks = PieceAttacks::rookAttacks(square, board.occupied);
		_attacks[Us][ROOK] |= attacks;
		_attacksMultiple[Us] |= _attacksAll[Us] & attacks;
		_attacksAll[Us] |= attacks;
	}

	piece = { QUEEN, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateQueen<Us>(square);

		const U64 attacks = PieceAttacks::queenAttacks(square, board.occupied);
		_attacks[Us][QUEEN] |= attacks;
		_attacksMultiple[Us] |= _attacksAll[Us] & attacks;
		_attacksAll[Us] |= attacks;
	}

	return score + pawnScore;
}

template <Color Us>
Score Evaluation::evaluateAttacks() const noexcept
{
	constexpr Color Them = ~Us;
	Score score{};

	const U64 nonPawnEnemies = board.allPieces[Them] & ~board.getType(PAWN, Us);
	const U64 stronglyProtected = _attacks[Them][PAWN]
								  | (_attacksMultiple[Them] & ~_attacksMultiple[Us]);
	const U64 weak = board.allPieces[Them] & ~stronglyProtected & _attacksAll[Us];
	const U64 defended = nonPawnEnemies & stronglyProtected;
	const U64 safe = ~_attacksAll[Them] | _attacksAll[Us];

	if (weak | defended)
	{
		U64 minorThreats = (defended | weak) & (_attacks[Us][KNIGHT] | _attacks[Us][BISHOP]) & board.allPieces[Them];
		while (minorThreats)
			score += MINOR_THREATS[board.getPiece(findNextSquare(minorThreats)).type()];

		U64 rookThreats = weak & _attacks[Us][ROOK] & board.allPieces[Them];
		while (rookThreats)
			score += ROOK_THREATS[board.getPiece(findNextSquare(rookThreats)).type()];

		if (weak & _attacks[Us][KING])
			score += THREAT_BY_KING;

		const U64 hangingPieces = ~_attacksAll[Them] | (nonPawnEnemies & _attacksMultiple[Us]);
		score += HANGING * popCount(weak & hangingPieces);

		score += WEAK_QUEEN_PROTECTION * popCount(weak & _attacks[Them][QUEEN]);
	}

	// Bonus for threats on the next moves against enemy queen
	if (board.pieceCount[Piece{ QUEEN, Them }])
	{
		const byte sq = board.pieceList[Piece{ QUEEN, Them }][0];
		const U64 safeSpots = _mobilityArea[Us] & ~stronglyProtected;

		const U64 knightAttacks = _attacks[Us][KNIGHT] & PieceAttacks::knightAttacks(sq);

		score += QUEEN_KNIGHT_THREAT * popCount(knightAttacks & safeSpots);

		const U64 sliderAttacks = (_attacks[Us][BISHOP] &
			PieceAttacks::bishopAttacks(sq, board.occupied))
			 | (_attacks[Us][ROOK] & PieceAttacks::rookAttacks(sq, board.occupied));

		score += QUEEN_SLIDER_THREAT * popCount(sliderAttacks & safe & _attacksMultiple[Us]);
	}

	const U64 restrictedMovement = _attacksAll[Them] & _attacksAll[Us] & ~stronglyProtected;
	score += RESTRICTED_PIECE_MOVEMENT * short(popCount(restrictedMovement));

	const U64 safePawnsAttacks =
		PieceAttacks::pawnAttacks<Us>(board.getType(PAWN, Us) & safe) & nonPawnEnemies;
	score += THREAT_BY_SAFE_PAWN * short(popCount(safePawnsAttacks));

	return score;
}

template <Color Us>
Score Evaluation::evaluatePawn(const byte square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr Dir ForwardDir = Us ? NORTH : SOUTH;
	constexpr Dir BehindDir = Us ? SOUTH : NORTH;
	constexpr byte Behind = Us ? -1 : 1;

	Score value = Psqt::BONUS[PAWN][square];

	const Pos pos{ square };
	const byte rank = (Us ? pos.y : 7u - pos.y) - 1u;

	const U64 adjacentFiles = getAdjacentFiles(square);
	const U64 opposed = board.getType(PAWN, Them) & getRay(ForwardDir, square);
	const U64 neighbours = board.getType(PAWN, Us) & adjacentFiles;
	const U64 connected = neighbours & getRank(square);
	const U64 support = neighbours & getRank(toSquare(pos.x + Behind, pos.y));
	const bool isDoubled = shift<BehindDir>(getSquare64(square)) & board.getType(PAWN, Us);

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
		U64 rays = getRay(ForwardDir, square);
		const U64 adjacentRays = shift<WEST>(rays) | shift<EAST>(rays);
		rays |= Us ? shift<NORTH>(adjacentRays) : shift<SOUTH>(adjacentRays);

		const bool isPassedPawn = !(rays & board.getType(PAWN, Them));

		if (isPassedPawn)
			value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

template <Color Us>
Score Evaluation::evaluateKnight(const byte square) const noexcept
{
	Score value = Psqt::BONUS[KNIGHT][square];

	const int mobility = popCount(PieceAttacks::knightAttacks(square) & _mobilityArea[Us]);
	value += KNIGHT_MOBILITY[mobility];

	return value;
}

template <Color Us>
Score Evaluation::evaluateBishop(const byte square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr U64 CenterFiles = FILE_D | FILE_E;
	constexpr U64 Center = CenterFiles & (RANK_4 | RANK_5);

	Score value = Psqt::BONUS[BISHOP][square];

	const int mobility = popCount(
		PieceAttacks::bishopAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += BISHOP_MOBILITY[mobility];

	// Long Diagonal Bishop
	const U64 centerAttacks =
		PieceAttacks::bishopAttacks(square, board.getType(PAWN, Them)) & Center;
	if (popCount(centerAttacks) > 1)
		value.mg += 45;

	return value;
}

template <Color Us>
Score Evaluation::evaluateRook(const byte square) const noexcept
{
	constexpr Color Them = ~Us;

	const Pos pos{ square };
	Score value = Psqt::BONUS[ROOK][square];

	const int mobility = popCount(
		PieceAttacks::rookAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += ROOK_MOBILITY[mobility];

	const U64 file = getFile(square);
	if (!(board.getType(PAWN, Us) & file))
		value += ROOK_ON_FILE[!(board.getType(PAWN, Them) & file)];
	else if (mobility <= 3)
	{
		const byte kx = col(board.pieceList[Piece{ KING, Us }][0]);

		if ((kx < 4) == (pos.x < kx))
			value -= TRAPPED_ROOK * (1 + !board.canCastle<Us>());
	}

	const U64 queens = board.getType(QUEEN, Us) | board.getType(QUEEN, Them);
	if (file & queens)
		value += ROOK_ON_QUEEN_FILE;

	return value;
}

template <Color Us>
Score Evaluation::evaluateQueen(const byte square) const noexcept
{
	Score value = Psqt::BONUS[QUEEN][square];

	const int mobility = popCount(
		PieceAttacks::queenAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += QUEEN_MOBILITY[mobility];

	if (const U64 initialPosition = FILE_D & (Us ? RANK_1 : RANK_8);
		(initialPosition & shiftedBoards[square]) == 0)
		value.mg -= 20;

	return value;
}

template <Color Us>
Score Evaluation::evaluateKing(const byte square) const noexcept
{
	Score value = Psqt::BONUS[KNIGHT][square];

	if (board.isCastled<Us>())
		value.mg += 57;
	else
	{
		const short count = short(board.canCastleKs<Us>()) + short(board.canCastleQs<Us>());
		value.mg += count * 19;
	}

	value += KING_PAWN_SHIELD *
			 popCount(Masks::PAWN_SHIELD[Us][square] & board.getType(PAWN, Us));

	return value;
}
