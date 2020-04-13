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
	 * Most of these values were imported from the Stockfish Chess Engine
	 */
	constexpr S PAWN_DOUBLED{ 11, 56 };
	constexpr S PAWN_ISOLATED{ 5, 15 };
	constexpr short PAWN_CONNECTED[]
	{
		0, 7, 8, 12, 29, 48, 86
	};
	constexpr S PASSED_PAWN_RANK[]
	{
		{ }, { 10, 28 }, { 17, 33 },  { 15, 41 }, {62, 72 }, { 168, 177 }, { 276, 260 }
	};

	constexpr S MINOR_PAWN_SHIELD{ 18,  3 };
	constexpr S MINOR_THREATS[]
	{
		{ }, { 6, 32 }, { 59, 41 }, { 79, 56 }, { 90, 119 }, { 79, 161 }, { }
	};
	constexpr S KING_PROTECTOR{ 7, 8 };
	constexpr S HANGING{ 69, 36 };
	constexpr S RESTRICTED_PIECE_MOVEMENT{ 7,  7 };
	constexpr S WEAK_QUEEN_PROTECTION{ 14,  0 };
	constexpr S THREAT_BY_KING{ 24, 89 };
	constexpr S THREAT_BY_SAFE_PAWN{ 173, 94 };

	constexpr S ROOK_ON_PAWN{ 10, 28 };
	constexpr S ROOK_ON_QUEEN_FILE{ 7, 6 };
	constexpr S TRAPPED_ROOK{ 52, 10 };
	constexpr S ROOK_ON_FILE[] =
	{
		{ 21, 4 }, { 47, 25 }
	};
	constexpr S ROOK_THREATS[]
	{
		{ }, { 3, 44 }, { 38, 71 }, { 38, 61 }, { 0, 38 }, { 51, 38 }, { 0, 0 }
	};
	constexpr S KING_PAWN_SHIELD{ 10, 0 };

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

PawnStructureTable Evaluation::_pawnTable{ 1 };

using namespace Bits;

namespace Masks
{
	static auto constexpr PAWN_SHIELD = []
	{
		std::array<std::array<U64, SQUARE_NB>, 2> array{};

		for (byte i{}; i < SQUARE_NB; ++i)
		{
			const U64 square = 1ull << i;

		    array[WHITE][i] = (square << 8 | (square << 7 & ~FILE_H) | (square << 9 & ~FILE_A)) & RANK_2;
			array[BLACK][i] = (square >> 8 | (square >> 7 & ~FILE_A) | (square >> 9 & ~FILE_H)) & RANK_7;
		}

		return array;
	}();
}

short Evaluation::evaluate(const Board &board) noexcept
{
	Stats::incrementBoardsEvaluated();

	Evaluation evaluator(board);

	const Score piecesScore = evaluator.evaluatePieces<WHITE>() - evaluator.evaluatePieces<BLACK>();
	const Score attacksScore = evaluator.evaluateAttacks<WHITE>() - evaluator.evaluateAttacks<BLACK>();
	Score finalScore = piecesScore - attacksScore;

	if (board.colorToMove)
	{
		finalScore += TEMPO_BONUS;

		if (Bits::getSquare64(board.getKingSq(BLACK)) & evaluator._attacksAll[WHITE])
			finalScore += CHECK_BONUS;
	} else
	{
		finalScore -= TEMPO_BONUS;

		if (Bits::getSquare64(board.getKingSq(WHITE)) & evaluator._attacksAll[BLACK])
			finalScore -= CHECK_BONUS;
	}
	
	const Phase phase = board.getPhase();
	return phase == Phase::MIDDLE ? finalScore.mg : finalScore.eg;
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

		const U64 attacks = PieceAttacks::getKingAttacks(square);
		_attacks[Us][KING] = attacks;
		_attacksAll[Us] = attacks;
	}

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
			{
				const byte square = board.pieceList[piece][pieceNumber];
				pawnScore += evaluatePawn<Us>(square);
			}

			_pawnTable.insert({ pawns, pawnScore });
		} else
			pawnScore = pawnsEntry.score;
	}

	{
		const U64 &pawns = board.getType(Us, PAWN);
		const U64 pawnsAttacks = PieceAttacks::getPawnAttacks<Us>(pawns);
		const U64 doublePawnsAttacks = PieceAttacks::getDoublePawnAttacks<Us>(pawns);

		_attacks[Us][PAWN] = pawnsAttacks;
		_attacksMultiple[Us] = doublePawnsAttacks | (doublePawnsAttacks & _attacksAll[Us]);
		_attacksAll[Us] |= pawnsAttacks;
	}

	_mobilityArea[Us] = ~board.allPieces[Us] & ~PieceAttacks::getPawnAttacks<Them>(board.getType(Them, PAWN));

	const auto knightBishop = [&] (const byte square)
	{
		constexpr Dir Behind = Us ? Dir::SOUTH : Dir::NORTH;

		if (getSquare64(square) & shift<Behind>(board.getType(Us, PAWN)))
			score += MINOR_PAWN_SHIELD;

		//score -= KING_PROTECTOR * getDistance(square, board.getKingSq(Us));
	};

	Piece piece{ KNIGHT, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateKnight<Us>(square);
		knightBishop(square);
		
		const U64 attacks = PieceAttacks::getKnightAttacks(square);
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

		const U64 attacks = PieceAttacks::getBishopAttacks(square, board.occupied);
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
		
		const U64 attacks = PieceAttacks::getRookAttacks(square, board.occupied);
		_attacks[Us][ROOK] |= attacks;
		_attacksMultiple[Us] |= _attacksAll[Us] & attacks;
		_attacksAll[Us] |= attacks;
	}

	piece = { QUEEN, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		score += evaluateQueen<Us>(square);
		
		const U64 attacks = PieceAttacks::getQueenAttacks(square, board.occupied);
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

    const U64 nonPawnEnemies = board.allPieces[Them] & ~board.getType(Us, PAWN);
    const U64 stronglyProtected = _attacks[Them][PAWN]
                       | (_attacksMultiple[Them] & ~_attacksMultiple[Us]);
    const U64 defended = nonPawnEnemies & stronglyProtected;
    const U64 weak = board.allPieces[Them] & ~stronglyProtected & _attacksAll[Us];
	const U64 safe = ~_attacksAll[Them] | _attacksAll[Us];

    if (defended || weak)
    {
        U64 minorThreats = (defended | weak) & (_attacks[Us][KNIGHT] | _attacks[Us][BISHOP]);
        while (minorThreats)
            score += MINOR_THREATS[board.getPiece(findNextSquare(minorThreats)).type()];

        U64 rookThreats = weak & _attacks[Us][ROOK];
        while (rookThreats)
            score += ROOK_THREATS[board.getPiece(findNextSquare(rookThreats)).type()];

        if (weak & _attacks[Us][KING])
            score += THREAT_BY_KING;

		const U64 hangingPieces = ~_attacksAll[Them] | (nonPawnEnemies & _attacksMultiple[Us]);
		score += HANGING * popCount(weak & hangingPieces);

        score += WEAK_QUEEN_PROTECTION * popCount(weak & _attacks[Them][QUEEN]);
    }

    const U64 restrictedMovement = _attacksAll[Them] & _attacksAll[Us] & ~stronglyProtected;
    score += RESTRICTED_PIECE_MOVEMENT * short(popCount(restrictedMovement));

	const U64 safePawnsAttacks = PieceAttacks::getPawnAttacks<Us>(board.getType(Us, PAWN) & safe) & nonPawnEnemies;
    score += THREAT_BY_SAFE_PAWN * short(popCount(safePawnsAttacks));

	return score;
}

template <Color Us>
Score Evaluation::evaluatePawn(const byte square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr Dir ForwardDir = Us ? NORTH : SOUTH;
	constexpr byte Behind = Us ? -1 : 1;
	constexpr Piece Piece{ PAWN, Us };

	Score value = Psqt::BONUS[PAWN][square];

	const Pos pos(square);
	const byte rank = (Us ? pos.y : 7u - pos.y) - 1u;

	const U64 adjacentFiles = getAdjacentFiles(square);
	const U64 opposed = board.getType(Them, PAWN) & getRay(ForwardDir, square);
	const U64 neighbours = board.getType(Piece) & adjacentFiles;
	const U64 connected = neighbours & getRank(square);
	const U64 support = neighbours & getRank(toSquare(pos.x + Behind, pos.y));
	const bool isDoubled = board.at(pos.x, pos.y - Behind) == Piece;

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

		const bool isPassedPawn = !bool(rays & board.getType(Them, PAWN));

		if (isPassedPawn)
			value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

template <Color Us>
Score Evaluation::evaluateKnight(const byte square) const noexcept
{
	Score value = Psqt::BONUS[KNIGHT][square];

	const int mobility = popCount(PieceAttacks::getKnightAttacks(square) & _mobilityArea[Us]);
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

	const int mobility = popCount(PieceAttacks::getBishopAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += BISHOP_MOBILITY[mobility];

	// Long Diagonal Bishop
	const U64 centerAttacks = PieceAttacks::getBishopAttacks(square, board.getType(Them, PAWN)) & Center;
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

	const int mobility = popCount(PieceAttacks::getRookAttacks(square, board.occupied) & _mobilityArea[Us]);
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
Score Evaluation::evaluateQueen(const byte square) const noexcept
{
	Score value = Psqt::BONUS[QUEEN][square];

	const int mobility = popCount(PieceAttacks::getQueenAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += QUEEN_MOBILITY[mobility];

	if (const U64 initialPosition = FILE_D & (Us ? RANK_1 : RANK_8);
		(initialPosition & shiftedBoards[square]) == 0)
		value.mg -= 40;

	return value;
}

template <Color Us>
Score Evaluation::evaluateKing(const byte square) const noexcept
{
	Score value = Psqt::BONUS[KNIGHT][square];

	if (board.isCastled(Us))
		value.mg += 57;
	else {
		const short count = short(board.canCastleKs(Us)) + short(board.canCastleQs(Us));
		value.mg += count * 19;
	}
	
	value += KING_PAWN_SHIELD * 
		popCount(Masks::PAWN_SHIELD[Us][square] & board.getType(Us, PAWN));

	return value;
}
