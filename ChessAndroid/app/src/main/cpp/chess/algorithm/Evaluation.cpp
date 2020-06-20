#include "Evaluation.h"

#include <sstream>
#include <iomanip>

#include "../Stats.h"
#include "../Board.h"
#include "../Psqt.h"

namespace
{
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
			{}, { 10, 28 }, { 17, 33 }, { 15, 41 }, { 62, 72 }, { 168, 177 }, { 276, 260 }
		};

	constexpr Score MINOR_PAWN_SHIELD{ 18, 3 };
	constexpr Score THREATS_BY_MINOR[]
		{
			{}, { 6, 32 }, { 59, 41 }, { 79, 56 }, { 90, 119 }, { 79, 161 }, {}
		};
	constexpr Score KING_PROTECTOR{ 4, 6 };
	constexpr Score HANGING{ 69, 36 };
	constexpr Score RESTRICTED_PIECE_MOVEMENT{ 7, 7 };
	constexpr Score WEAK_QUEEN_PROTECTION{ 14, 0 };
	constexpr Score THREAT_BY_KING{ 24, 89 };
	constexpr Score THREAT_BY_SAFE_PAWN{ 173, 94 };
	constexpr Score QUEEN_THREAT_BY_KNIGHT{ 16, 11 };
	constexpr Score QUEEN_THREAT_BY_SLIDER{ 59, 18 };

	constexpr Score ROOK_ON_QUEEN_FILE{ 7, 6 };
	constexpr Score TRAPPED_ROOK{ 52, 10 };
	constexpr Score ROOK_ON_FILE[] =
		{
			{ 21, 4 }, { 47, 25 }
		};
	constexpr Score THREAT_BY_ROOK[]
		{
			{}, { 3, 44 }, { 38, 71 }, { 38, 61 }, { 0, 38 }, { 51, 38 }, {}
		};

	// King Safety
	constexpr Score KING_PAWN_SHIELD{ 26, 4 };

	// Mobility
	constexpr Score KNIGHT_MOBILITY[]
		{
			{ -62, -81 }, { -53, -56 }, { -12, -30 }, { -4, -14 }, { 3, 8 }, { 13, 15 },
			{ 22, 23 }, { 28, 27 }, { 33, 33 }
		};
	constexpr Score BISHOP_MOBILITY[]
		{
			{ -48, -59 }, { -20, -23 }, { 16, -3 }, { 26, 13 }, { 38, 24 }, { 51, 42 },
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

PawnStructureTable _pawnTable{ 8 };

using namespace Bits;

constexpr auto MASK_PAWN_SHIELD = []
{
	std::array<std::array<U64, SQUARE_NB>, 2> array{};

	for (byte i{}; i < SQUARE_NB; ++i)
	{
		const U64 bb = getSquare64(i);

		array[WHITE][i] = (bb << 8 | (bb << 7 & ~FILE_H) | (bb << 9 & ~FILE_A)) & RANK_2;
		array[BLACK][i] = (bb >> 8 | (bb >> 7 & ~FILE_A) | (bb >> 9 & ~FILE_H)) & RANK_7;
	}

	return array;
}();

template<bool Trace>
struct Eval
{
	explicit Eval(const Board &board) noexcept
		: board(board)
	{
		if constexpr (Trace)
			trace = new Evaluation::Trace{};
	}

	~Eval() noexcept
	{
		if constexpr (Trace)
			delete trace;
	}

	int computeValue() noexcept;

	const auto &getTrace() const noexcept
	{
		static_assert(Trace);
		return *trace;
	}

private:
	template<Color Us>
	Score evaluatePieces() noexcept;
	template<Color Us>
	Score evaluateAttacks() const noexcept;
	template<Color Us>
	Score evaluatePawn(byte square) const noexcept;
	template<Color Us>
	Score evaluateKnight(byte square) const noexcept;
	template<Color Us>
	Score evaluateBishop(byte square) const noexcept;
	template<Color Us>
	Score evaluateRook(byte square) const noexcept;
	template<Color Us>
	Score evaluateQueen(byte square) const noexcept;
	template<Color Us>
	Score evaluateKing() const noexcept;

	const Board &board;
	Evaluation::Trace *trace = nullptr;
	std::array<std::array<U64, 6>, 2> _pieceAttacks{}; // No King
	std::array<U64, 2> _attacksMultiple{};
	std::array<U64, 2> _allAttacks{};
	std::array<U64, 2> _mobilityArea{};

	const std::array<U64, 2> _kingRing{ Bits::getSquare64(board.getKingSq<WHITE>()),
										Bits::getSquare64(board.getKingSq<BLACK>()) };
	std::array<int, 2> _kingAttacksCount{};
	std::array<int, 2> _kingAttackersCount{};
	std::array<int, 2> _kingAttackersWeight{};
};

int Evaluation::value(const Board &board) noexcept
{
	Stats::incBoardsEvaluated();
	return Eval<false>{ board }.computeValue();
}

int Evaluation::invertedValue(const Board &board) noexcept
{
	const int result = value(board);
	return board.colorToMove ? result : -result;
}

std::string Evaluation::traceValue(const Board &board)
{
	Eval<true> eval{ board };
	const int result = eval.computeValue();

	const auto &trace = eval.getTrace();
	std::ostringstream output;

	const auto element = [&](const std::string_view name, const std::array<Score, 2> &elem)
	{
		constexpr int MaxLetters = 24;
		constexpr int MaxDigits = 5;

		output << "| "
			   << std::setfill(' ') << std::setw(MaxLetters) << name << " | "
			   << std::setw(MaxDigits) << elem[WHITE].mg << ' '
			   << std::setw(MaxDigits) << elem[WHITE].eg << " | "
			   << std::setw(MaxDigits) << elem[BLACK].mg << ' '
			   << std::setw(MaxDigits) << elem[BLACK].eg << " | "
			   << std::setw(MaxDigits) << elem[WHITE].mg - elem[BLACK].mg << ' '
			   << std::setw(MaxDigits) << elem[WHITE].eg - elem[BLACK].eg << " |\n";
	};

	const auto Separator = "+--------------------------+-------------+-------------+-------------+\n";
	output << Separator
		   << "|                          |    WHITE    |    BLACK    |    TOTAL    |\n"
		   << "|                          |   MG   EG   |   MG   EG   |   MG   EG   |\n"
		   << Separator;

	element("Pawns", trace.pawns);
	element("Knights", trace.knights);
	element("Bishops", trace.bishops);
	element("Rooks", trace.rooks);
	element("Queens", trace.queen);
	element("King", trace.king);
	element("Mobility", trace.mobility);
	element("Pieces Total", trace.piecesTotal);
	element("King Protectors", trace.kingProtector);
	element("Minors Pawn Shield", trace.minorPawnShield);
	element("Threats By Minor", trace.threatsByMinor);
	element("Threats By Rook", trace.threatsByRook);
	element("Threats By King", trace.threatsByKing);
	element("Threats By Safe-Pawns", trace.threatBySafePawn);
	element("Hanging Pieces", trace.piecesHanging);
	element("Protection by Weak Queen", trace.weakQueenProtection);
	element("Queen Threat By Knight", trace.queenThreatByKnight);
	element("Queen Threat By Slider", trace.queenThreatBySlider);
	element("Restricted Movement", trace.restrictedMovement);
	element("Attacks Total", trace.attacksTotal);

	output << Separator;
	element("Total", trace.total);
	output << "|                   Scaled |             |             | "
		   << std::setw(11) << result << " |\n"
		   << Separator;

	return output.str();
}

template<bool Trace>
int Eval<Trace>::computeValue() noexcept
{
	Score totalWhite = evaluatePieces<WHITE>();
	Score totalBlack = evaluatePieces<BLACK>();

	const auto kingScoreWhite = evaluateKing<WHITE>();
	const auto kingScoreBlack = evaluateKing<BLACK>();

	totalWhite += kingScoreWhite + evaluateAttacks<WHITE>();
	totalBlack += kingScoreBlack + evaluateAttacks<BLACK>();

	if constexpr (Trace)
	{
		trace->king[WHITE] = kingScoreWhite;
		trace->king[BLACK] = kingScoreBlack;
		trace->total[WHITE] = totalWhite;
		trace->total[BLACK] = totalBlack;
	}

	Score score = totalWhite - totalBlack;

	if (board.colorToMove)
		score += Evaluation::TEMPO_BONUS;
	else
		score -= Evaluation::TEMPO_BONUS;

	const Phase phase = board.getPhase();
	return (score.mg * phase + (score.eg * (128 - phase))) / 128;
}

template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluatePieces() noexcept
{
	constexpr Color Them = ~Us;
	Score score;

	_allAttacks[Us] = _kingRing[Us];

	// Bonuses for Knights and Bishops
	const auto knightBishopBonus = [&](const byte square)
	{
		constexpr Dir Behind = Us ? Dir::SOUTH : Dir::NORTH;

		const auto kingProtectorScore =
			KING_PROTECTOR * getDistanceBetween(square, board.getKingSq<Us>());
		score -= kingProtectorScore;
		if constexpr (Trace)
			trace->kingProtector[Us] -= kingProtectorScore;
		const U64 bb = getSquare64(square);
		const U64 pawns = shift<Behind>(board.getType(PAWN, Us));

		if (bb & pawns)
		{
			score += MINOR_PAWN_SHIELD;

			if constexpr (Trace)
				trace->minorPawnShield[Us] += MINOR_PAWN_SHIELD;
		}
	};

	const auto updateAttacks = [&] (const PieceType type, const U64 attacks)
	{
		_pieceAttacks[Us][type] |= attacks;
		_attacksMultiple[Us] |= _allAttacks[Us] & attacks;
		_allAttacks[Us] |= attacks;
	};

	const auto updateKingAttacks = [&](const PieceType type, const U64 attacks)
	{
		/*constexpr Color Them = ~Us;
		if (attacks & _kingRing[Them])
		{
			++_kingAttackersCount[Us];
			_kingAttackersWeight[Us] += KING_ATTACK_WEIGHT[type];
			_kingAttacksCount[Us] += popCount(attacks & _kingRing[Them]);
		}*/
	};

	Score pawnScore;

	{
		U64 pawns = board.getType(PAWN, Us);

		if constexpr (Us == BLACK)
			pawns = flipVertical(pawns);

		const auto pawnsEntry = _pawnTable[pawns];

		// Don't use the Pawn Structure Table if we are Tracing the Eval
		if (Trace || pawnsEntry.pawns != pawns)
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

	// Pawns
	{
		const U64 &pawns = board.getType(PAWN, Us);
		const U64 pawnsAttacks = Attacks::pawnAttacks<Us>(pawns);
		const U64 doublePawnsAttacks = Attacks::pawnDoubleAttacks<Us>(pawns);

		_pieceAttacks[Us][PAWN] = pawnsAttacks;
		_attacksMultiple[Us] = doublePawnsAttacks | (doublePawnsAttacks & _allAttacks[Us]);
		_allAttacks[Us] |= pawnsAttacks;

		updateKingAttacks(PAWN, pawnsAttacks);
	}

	_mobilityArea[Us] =
		~board.allPieces[Us] & ~Attacks::pawnAttacks<Them>(board.getType(PAWN, Them));

	Piece piece{ KNIGHT, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		const auto knightScore = evaluateKnight<Us>(square);
		knightBishopBonus(square);

		if constexpr (Trace)
			trace->knights[Us] += knightScore;
		score += knightScore;

		const U64 attacks = Attacks::knightAttacks(square);
		updateAttacks(KNIGHT, attacks);
		updateKingAttacks(KNIGHT, attacks);
	}

	piece = { BISHOP, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		const auto bishopScore = evaluateBishop<Us>(square);
		knightBishopBonus(square);

		if constexpr (Trace)
			trace->bishops[Us] += bishopScore;
		score += bishopScore;

		const U64 attacks = Attacks::bishopAttacks(square, board.occupied);
		updateAttacks(BISHOP, attacks);
		updateKingAttacks(BISHOP, attacks);
	}

	if (board.pieceCount[piece] >= 2)
		score += 40;

	piece = { ROOK, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		const auto rookScore = evaluateRook<Us>(square);

		if constexpr (Trace)
			trace->rooks[Us] += rookScore;
		score += rookScore;

		const U64 attacks = Attacks::rookAttacks(square, board.occupied);
		updateAttacks(ROOK, attacks);
		updateKingAttacks(ROOK, attacks);
	}

	piece = { QUEEN, Us };
	for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const byte square = board.pieceList[piece][pieceNumber];
		const auto queenScore = evaluateQueen<Us>(square);

		if constexpr (Trace)
			trace->queen[Us] += queenScore;
		score += queenScore;

		const U64 attacks = Attacks::queenAttacks(square, board.occupied);
		updateAttacks(QUEEN, attacks);
		updateKingAttacks(QUEEN, attacks);
	}

	if constexpr (Trace)
	{
		trace->pawns[Us] = pawnScore;
		trace->piecesTotal[Us] = score + pawnScore;
	}

	return score + pawnScore;
}

/*
 * Needs to be called after both evaluatePieces<WHITE>() and evaluatePieces<BLACK>()
 */
template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluateAttacks() const noexcept
{
	constexpr Color Them = ~Us;
	Score totalValue{};

	const U64 nonPawnEnemies = board.allPieces[Them] & ~board.getType(PAWN, Us);
	const U64 stronglyProtected = _pieceAttacks[Them][PAWN]
								  | (_attacksMultiple[Them] & ~_attacksMultiple[Us]);
	const U64 poorlyDefended = board.allPieces[Them] & ~stronglyProtected & _allAttacks[Us];
	const U64 defended = nonPawnEnemies & stronglyProtected;
	const U64 safe = ~_allAttacks[Them] | _allAttacks[Us];

	if (poorlyDefended | defended)
	{
		U64 minorThreats =
			(defended | poorlyDefended) & (_pieceAttacks[Us][KNIGHT] | _pieceAttacks[Us][BISHOP]) &
			board.allPieces[Them];
		while (minorThreats)
		{
			const auto value = THREATS_BY_MINOR[board.getPiece(popLsb(minorThreats)).type()];
			totalValue += value;
			if constexpr (Trace)
				trace->threatsByMinor[Us] += value;
		}

		U64 rookThreats = poorlyDefended & _pieceAttacks[Us][ROOK] & board.allPieces[Them];
		while (rookThreats)
		{
			const auto value = THREAT_BY_ROOK[board.getPiece(popLsb(rookThreats)).type()];
			totalValue += value;
			if constexpr (Trace)
				trace->threatsByRook[Us] += value;
		}

		if (poorlyDefended & _kingRing[Us])
		{
			totalValue += THREAT_BY_KING;
			if constexpr (Trace)
				trace->threatsByKing[Us] += THREAT_BY_KING;
		}

		const U64 hangingPieces = ~_allAttacks[Them] | (nonPawnEnemies & _attacksMultiple[Us]);
		const auto hangingScore = HANGING * popCount(poorlyDefended & hangingPieces);

		totalValue += hangingScore;

		const auto weakQueenScore =
			WEAK_QUEEN_PROTECTION * popCount(poorlyDefended & _pieceAttacks[Them][QUEEN]);
		totalValue += weakQueenScore;

		if constexpr (Trace)
		{
			trace->piecesHanging[Us] = hangingScore;
			trace->weakQueenProtection[Us] = weakQueenScore;
		}
	}

	// Bonus for threats on the next moves against enemy queen
	if (board.pieceCount[Piece{ QUEEN, Them }])
	{
		const byte sq = board.pieceList[Piece{ QUEEN, Them }][0];
		const U64 safeSpots = _mobilityArea[Us] & ~stronglyProtected;

		const U64 knightAttacks = _pieceAttacks[Us][KNIGHT] & Attacks::knightAttacks(sq);
		const U64 sliderAttacks = (_pieceAttacks[Us][BISHOP] &
								   Attacks::bishopAttacks(sq, board.occupied))
								  | (_pieceAttacks[Us][ROOK] &
									 Attacks::rookAttacks(sq, board.occupied));

		const auto knightAttacksScore =
			QUEEN_THREAT_BY_KNIGHT * popCount(knightAttacks & safeSpots);
		const auto sliderAttacksScore =
			QUEEN_THREAT_BY_SLIDER * popCount(sliderAttacks & safe & _attacksMultiple[Us]);

		totalValue += knightAttacksScore + sliderAttacksScore;
		if constexpr (Trace)
		{
			trace->queenThreatByKnight[Us] = knightAttacksScore;
			trace->queenThreatBySlider[Us] = sliderAttacksScore;
		}
	}

	const U64 safePawnsAttacks =
		Attacks::pawnAttacks<Us>(board.getType(PAWN, Us) & safe) & nonPawnEnemies;
	const auto safePawnThreatScore = THREAT_BY_SAFE_PAWN * popCount(safePawnsAttacks);
	totalValue += safePawnThreatScore;

	const U64 restrictedMovement = _allAttacks[Them] & _allAttacks[Us] & ~stronglyProtected;
	const Score restrictedMovementScore = RESTRICTED_PIECE_MOVEMENT * popCount(restrictedMovement);
	totalValue += restrictedMovementScore;

	if constexpr (Trace)
	{
		trace->threatBySafePawn[Us] = safePawnThreatScore;
		trace->restrictedMovement[Us] = restrictedMovementScore;
		trace->attacksTotal[Us] = totalValue;
	}

	return totalValue;
}

template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluatePawn(const byte square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr Dir ForwardDir = Us ? NORTH : SOUTH;
	constexpr Dir BehindDir = Us ? SOUTH : NORTH;

	Score value = PSQT[PAWN][square];

	const U64 bb = getSquare64(square);
	const byte rank = (Us ? row(square) : 7u - row(square)) - 1u;

	const U64 adjacentFiles = getAdjacentFiles(square);
	const U64 opposed = board.getType(PAWN, Them) & getRay(ForwardDir, square);
	const U64 neighbours = board.getType(PAWN, Us) & adjacentFiles;
	const U64 connected = neighbours & getRank(square);
	const U64 support = neighbours & getRank(bitScanForward(shift<BehindDir>(bb)));
	const bool isDoubled = shift<BehindDir>(bb) & board.getType(PAWN, Us);

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
		rays |= shift<WEST>(rays) | shift<EAST>(rays);

		const bool isPassedPawn = !(rays & board.getType(PAWN, Them));

		if (isPassedPawn)
			value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluateKnight(const byte square) const noexcept
{
	Score value = PSQT[KNIGHT][square];

	const int mobility = popCount(Attacks::knightAttacks(square) & _mobilityArea[Us]);
	value += KNIGHT_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	return value;
}

template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluateBishop(const byte square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr U64 CenterFiles = FILE_D | FILE_E;
	constexpr U64 Center = CenterFiles & (RANK_4 | RANK_5);

	Score value = PSQT[BISHOP][square];

	const int mobility = popCount(
		Attacks::bishopAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += BISHOP_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	// Long Diagonal Bishop
	const U64 centerAttacks =
		Attacks::bishopAttacks(square, board.getType(PAWN, Them)) & Center;
	if (popCount(centerAttacks) > 1)
		value.mg += 45;

	return value;
}

template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluateRook(const byte square) const noexcept
{
	constexpr Color Them = ~Us;

	Score value = PSQT[ROOK][square];

	const int mobility = popCount(
		Attacks::rookAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += ROOK_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	const U64 file = getFile(square);
	if (!(board.getType(PAWN, Us) & file))
		value += ROOK_ON_FILE[!(board.getType(PAWN, Them) & file)];
	else if (mobility <= 3)
	{
		const byte kx = col(board.getKingSq<Us>());

		if ((kx < 4) == (col(square) < kx))
			value -= TRAPPED_ROOK * (1 + !board.canCastle<Us>());
	}

	const U64 queens = board.getType(QUEEN, Us) | board.getType(QUEEN, Them);
	if (file & queens)
		value += ROOK_ON_QUEEN_FILE;

	return value;
}

template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluateQueen(const byte square) const noexcept
{
	Score value = PSQT[QUEEN][square];

	const int mobility = popCount(
		Attacks::queenAttacks(square, board.occupied) & _mobilityArea[Us]);
	value += QUEEN_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	constexpr U64 InitialPosition = FILE_D & (Us ? RANK_1 : RANK_8);
	if ((InitialPosition & getSquare64(square)) == 0)
		value.mg -= 14;

	return value;
}

/*
 * Needs to be called after both evaluatePieces<WHITE>() and evaluatePieces<BLACK>()
 */
template<bool Trace>
template<Color Us>
Score Eval<Trace>::evaluateKing() const noexcept
{
	// constexpr Color Them = ~Us;
	const byte square = board.getKingSq<Us>();

	Score value = PSQT[KNIGHT][square];

	if (board.isCastled<Us>())
		value.mg += 70;
	else
	{
		const short count = short(board.canCastleKs<Us>()) + short(board.canCastleQs<Us>());
		value.mg += count * 20;
	}

	value += KING_PAWN_SHIELD *
			 popCount(MASK_PAWN_SHIELD[Us][square] & board.getType(PAWN, Us));

	return value;
}
