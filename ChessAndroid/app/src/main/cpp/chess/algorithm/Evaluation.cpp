#include "Evaluation.h"

#include <sstream>
#include <iomanip>

#include "../Stats.h"
#include "../Psqt.h"
#include "../PawnStructureTable.h"

namespace
{
	/*
	 * Most of these values were imported from the Stockfish Chess Engine
	 */
	constexpr Score PAWN_DOUBLED{ 10, 55 };
	constexpr Score PAWN_ISOLATED{ 3, 15 };
	constexpr short PAWN_CONNECTED[]{ 0, 5, 7, 11, 24, 48, 86 };
	constexpr Score PASSED_PAWN_RANK[] = {
		{}, { 7, 27 }, { 16, 32 }, { 17, 40 }, { 64, 71 }, { 170, 174 }, { 278, 262 }
	};

	constexpr Score MINOR_PAWN_SHIELD{ 18, 3 };
	constexpr Score THREATS_BY_MINOR[] = {
		{}, { 5, 32 }, { 55, 41 }, { 77, 56 }, { 89, 119 }, { 79, 162 }, {}
	};
	constexpr Score KING_PROTECTOR[] = {{ 8, 9 }, { 6, 9 }};
	constexpr Score HANGING{ 69, 36 };
	constexpr Score RESTRICTED_PIECE_MOVEMENT{ 7, 7 };
	constexpr Score WEAK_QUEEN_PROTECTION{ 14, 0 };
	constexpr Score THREAT_BY_KING{ 24, 89 };
	constexpr Score THREAT_BY_SAFE_PAWN{ 173, 94 };
	constexpr Score QUEEN_THREAT_BY_KNIGHT{ 16, 11 };
	constexpr Score QUEEN_THREAT_BY_SLIDER{ 60, 18 };
	constexpr Score BISHOP_XRAY_PAWNS{ 4, 5 };
	constexpr Score BISHOP_PAWNS[] = {
		{ 3, 8 }, { 3, 9 }, { 2, 8 }, { 3, 8 }
	};
	constexpr Score UNCONTESTED_OUTPOST{ 1, 10 };

	constexpr Score TRAPPED_ROOK{ 55, 13 };
	constexpr Score ROOK_ON_FILE[] = {
		{ 19, 7 }, { 48, 27 }
	};
	constexpr Score THREAT_BY_ROOK[] = {
		{}, { 3, 44 }, { 37, 68 }, { 42, 60 }, { 0, 39 }, { 58, 43 }, {}
	};

	constexpr Score KING_RING_THREAT_BISHOP{ 24, 0 };
	constexpr Score KING_RING_THREAT_ROOK{ 16, 0 };

	// King Safety
	constexpr Score KING_PAWN_SHIELD{ 26, 4 };

	// Mobility
	constexpr Score KNIGHT_MOBILITY[] = {
		{ -62, -79 }, { -53, -57 }, { -12, -31 }, { -3, -17 }, { 3, 7 },
		{ 12, 13 }, { 21, 16 }, { 28, 21 }, { 37, 26 }
	};
	constexpr Score BISHOP_MOBILITY[] = {
		{ -47, -59 }, { -20, -25 }, { 14, -8 }, { 29, 12 }, { 39, 21 },
		{ 53, 40 }, { 53, 56 }, { 60, 58 }, { 62, 65 }, { 69, 72 }, { 78, 78 },
		{ 83, 87 }, { 91, 88 }, { 96, 98 }
	};
	constexpr Score ROOK_MOBILITY[] = {
		{ -60, -82 }, { -24, -15 }, { 0, 17 }, { 3, 43 }, { 4, 72 },
		{ 14, 100 }, { 20, 102 }, { 30, 122 }, { 41, 133 }, { 41, 139 },
		{ 41, 153 }, { 45, 160 }, { 57, 165 }, { 58, 170 }, { 67, 175 }
	};
	constexpr Score QUEEN_MOBILITY[] = {
		{ -29, -49 }, { -16, -29 }, { -8, -8 }, { -8, 17 }, { 18, 39 }, { 25, 54 },
		{ 23, 59 }, { 37, 73 }, { 41, 76 }, { 54, 95 }, { 65, 95 }, { 68, 101 },
		{ 69, 124 }, { 70, 128 }, { 70, 132 }, { 70, 133 }, { 71, 136 }, { 72, 140 },
		{ 74, 147 }, { 76, 149 }, { 90, 153 }, { 104, 169 }, { 105, 171 }, { 106, 171 },
		{ 112, 178 }, { 114, 185 }, { 114, 187 }, { 119, 221 }
	};
}

static thread_local PawnStructureTable PawnTable{ 2 };

constexpr auto MASK_PAWN_SHIELD = []
{
	std::array<std::array<Bitboard, SQUARE_NB>, 2> array{};

	for (u8 square{}; square < SQUARE_NB; ++square)
	{
		const auto bb = Bitboard::fromSquare(toSquare(square));

		array[WHITE][square] = Bitboard{
			(bb << 8 | (bb << 7 & ~FILE_H) | (bb << 9 & ~FILE_A)) & RANK_2 };
		array[BLACK][square] = Bitboard{
			(bb >> 8 | (bb >> 7 & ~FILE_A) | (bb >> 9 & ~FILE_H)) & RANK_7 };
	}

	return array;
}();

template <bool Trace>
struct Eval
{
	explicit Eval(const Board &board) noexcept
		: board(board)
	{
		if constexpr (Trace)
			trace = new Evaluation::Trace{};

		const auto wKingSq = board.getKingSq<WHITE>();
		const auto bKingSq = board.getKingSq<BLACK>();
		const auto wKingRing = (Attacks::kingAttacks(wKingSq) | Bitboard::fromSquare(wKingSq))
							   & ~Attacks::pawnDoubleAttacks<WHITE>(board.getPieces(PAWN, WHITE));
		const auto bKingRing = (Attacks::kingAttacks(bKingSq) | Bitboard::fromSquare(bKingSq))
							   & ~Attacks::pawnDoubleAttacks<BLACK>(board.getPieces(PAWN, BLACK));
		_kingRing = { wKingRing, bKingRing };
	}

	~Eval() noexcept
	{
		if constexpr (Trace)
			delete trace;
	}

	int computeValue() noexcept;

	const auto &getTrace() const noexcept requires Trace
	{
		return *trace;
	}

private:
	template <Color Us>
	Score evaluatePieces() noexcept;
	template <Color Us>
	Score evaluateAttacks() const noexcept;
	template <Color Us>
	Score evaluatePawn(Square square) const noexcept;
	template <Color Us>
	Score evaluateKnight(Square square) const noexcept;
	template <Color Us>
	Score evaluateBishop(Square square) const noexcept;
	template <Color Us>
	Score evaluateRook(Square square) const noexcept;
	template <Color Us>
	Score evaluateQueen(Square square) const noexcept;
	template <Color Us>
	Score evaluateKing() const noexcept;

	const Board &board;
	Evaluation::Trace *trace = nullptr;
	std::array<std::array<Bitboard, 6>, 2> _pieceAttacks{}; // No King
	std::array<Bitboard, 2> _attacksMultiple{};
	std::array<Bitboard, 2> _allAttacks{};
	std::array<Bitboard, 2> _mobilityArea{};

	std::array<Bitboard, 2> _kingRing{};
	// TODO
	std::array<i32, 2> _kingAttacksCount{};
	std::array<i32, 2> _kingAttackersCount{};
	std::array<i32, 2> _kingAttackersWeight{};
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

template <bool Trace>
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

template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluatePieces() noexcept
{
	constexpr Color Them = ~Us;
	Score score;

	_allAttacks[Us] = _kingRing[Us];

	// Bonuses for Knights and Bishops
	const auto knightBishopBonus = [&](const Square square)
	{
		constexpr Dir Behind = Us ? Dir::SOUTH : Dir::NORTH;
		const bool isBishop = board.getPiece(square).type() == BISHOP;

		const auto kingProtectorScore =
			KING_PROTECTOR[isBishop] * Bits::getDistanceBetween(square, board.getKingSq<Us>());
		score -= kingProtectorScore;
		if constexpr (Trace)
			trace->kingProtector[Us] -= kingProtectorScore;

		const auto bb = Bitboard::fromSquare(square);
		const auto pawns = board.getPieces(PAWN, Us).shift<Behind>();

		if (bool(bb & pawns))
		{
			score += MINOR_PAWN_SHIELD;

			if constexpr (Trace)
				trace->minorPawnShield[Us] += MINOR_PAWN_SHIELD;
		}
	};

	const auto updateAttacks = [&](const PieceType type, const Bitboard attacks)
	{
		_pieceAttacks[Us][type] |= attacks;
		_attacksMultiple[Us] |= _allAttacks[Us] & attacks;
		_allAttacks[Us] |= attacks;
	};

	const auto updateKingAttacks = [&](const PieceType /*type*/, const Bitboard /*attacks*/)
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
		Bitboard pawns = board.getPieces(PAWN, Us);

		if constexpr (Us == BLACK)
			pawns = pawns.flipVertical();

		const auto pawnsEntry = PawnTable[pawns.value()];

		// Don't use the Pawn Structure Table if we are Tracing the Eval
		if (Trace || pawnsEntry.pawns != pawns)
		{
			constexpr Piece piece{ PAWN, Us };

			for (u8 pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
			{
				const auto square = toSquare(board.pieceList[piece][pieceNumber]);
				pawnScore += evaluatePawn<Us>(square);
			}

			PawnTable.insert({ pawns, pawnScore });
		} else
			pawnScore = pawnsEntry.score;
	}

	// Pawns
	{
		const auto pawns = board.getPieces(PAWN, Us);
		const auto pawnsAttacks = Attacks::pawnAttacks<Us>(pawns);
		const auto doublePawnsAttacks = Attacks::pawnDoubleAttacks<Us>(pawns);

		_pieceAttacks[Us][PAWN] = pawnsAttacks;
		_attacksMultiple[Us] = doublePawnsAttacks | (doublePawnsAttacks & _allAttacks[Us]);
		_allAttacks[Us] |= pawnsAttacks;

		updateKingAttacks(PAWN, pawnsAttacks);
	}

	_mobilityArea[Us] =
		~board.getPieces(Us) & ~Attacks::pawnAttacks<Them>(board.getPieces(PAWN, Them));

	Piece piece{ KNIGHT, Us };
	for (u8 pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const Square square = toSquare(board.pieceList[piece][pieceNumber]);
		const auto knightScore = evaluateKnight<Us>(square);
		knightBishopBonus(square);

		if constexpr (Trace)
			trace->knights[Us] += knightScore;
		score += knightScore;

		const auto attacks = Attacks::knightAttacks(square);
		updateAttacks(KNIGHT, attacks);
		updateKingAttacks(KNIGHT, attacks);
	}

	piece = { BISHOP, Us };
	for (u8 pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const auto square = toSquare(board.pieceList[piece][pieceNumber]);
		const auto bishopScore = evaluateBishop<Us>(square);
		knightBishopBonus(square);

		if constexpr (Trace)
			trace->bishops[Us] += bishopScore;
		score += bishopScore;

		const auto attacks = Attacks::bishopAttacks(square, board.occupied);
		updateAttacks(BISHOP, attacks);
		updateKingAttacks(BISHOP, attacks);
	}

	if (board.pieceCount[piece] >= 2)
		score += 40;

	piece = { ROOK, Us };
	for (u8 pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const auto square = toSquare(board.pieceList[piece][pieceNumber]);
		const auto rookScore = evaluateRook<Us>(square);

		if constexpr (Trace)
			trace->rooks[Us] += rookScore;
		score += rookScore;

		const auto attacks = Attacks::rookAttacks(square, board.occupied);
		updateAttacks(ROOK, attacks);
		updateKingAttacks(ROOK, attacks);
	}

	piece = { QUEEN, Us };
	for (u8 pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
	{
		const auto square = toSquare(board.pieceList[piece][pieceNumber]);
		const auto queenScore = evaluateQueen<Us>(square);

		if constexpr (Trace)
			trace->queen[Us] += queenScore;
		score += queenScore;

		const auto attacks = Attacks::queenAttacks(square, board.occupied);
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
template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluateAttacks() const noexcept
{
	constexpr Color Them = ~Us;
	Score totalValue{};

	const auto nonPawnEnemies = board.getPieces(Them) & ~board.getPieces(PAWN, Us);
	const auto stronglyProtected = _pieceAttacks[Them][PAWN]
								   | (_attacksMultiple[Them] & ~_attacksMultiple[Us]);
	const auto poorlyDefended = board.getPieces(Them) & ~stronglyProtected & _allAttacks[Us];
	const auto defended = nonPawnEnemies & stronglyProtected;

	if (poorlyDefended | defended)
	{
		Bitboard minorThreats =
			(defended | poorlyDefended) &
			(_pieceAttacks[Us][KNIGHT] | _pieceAttacks[Us][BISHOP]) &
			board.getPieces(Them);
		while (minorThreats)
		{
			const auto value = THREATS_BY_MINOR[board.getPiece(minorThreats.popLsb()).type()];
			totalValue += value;
			if constexpr (Trace)
				trace->threatsByMinor[Us] += value;
		}

		Bitboard rookThreats = poorlyDefended & _pieceAttacks[Us][ROOK] & board.getPieces(Them);
		while (rookThreats)
		{
			const auto value = THREAT_BY_ROOK[board.getPiece(rookThreats.popLsb()).type()];
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

		const auto hangingPieces = ~_allAttacks[Them] | (nonPawnEnemies & _attacksMultiple[Us]);
		const auto hangingScore = HANGING * (poorlyDefended & hangingPieces).popcount();

		totalValue += hangingScore;

		const auto weakQueenScore =
			WEAK_QUEEN_PROTECTION * (poorlyDefended & _pieceAttacks[Them][QUEEN]).popcount();
		totalValue += weakQueenScore;

		if constexpr (Trace)
		{
			trace->piecesHanging[Us] = hangingScore;
			trace->weakQueenProtection[Us] = weakQueenScore;
		}
	}

	// Bonus for threats on the next moves against enemy queen
	if (board.pieceCount[Piece{ QUEEN, Them }] == 1)
	{
		const bool imbalance =
			(board.pieceCount[Piece{ QUEEN, Them }] + board.pieceCount[Piece{ QUEEN, Us }]) ==
			1;

		const auto sq = toSquare(board.pieceList[Piece{ QUEEN, Them }][0]);
		const auto safeSpots =
			_mobilityArea[Us] & ~stronglyProtected & ~board.getPieces(PAWN, Them);

		const auto knightAttacks = _pieceAttacks[Us][KNIGHT] & Attacks::knightAttacks(sq);
		const auto sliderAttacks =
			(_pieceAttacks[Us][BISHOP] & Attacks::bishopAttacks(sq, board.occupied))
			| (_pieceAttacks[Us][ROOK] & Attacks::rookAttacks(sq, board.occupied));

		const auto knightAttacksScore =
			QUEEN_THREAT_BY_KNIGHT * (knightAttacks & safeSpots).popcount();
		const auto sliderAttacksScore =
			QUEEN_THREAT_BY_SLIDER *
			(sliderAttacks & safeSpots & _attacksMultiple[Us]).popcount() *
			(1 + imbalance);

		totalValue += knightAttacksScore + sliderAttacksScore;
		if constexpr (Trace)
		{
			trace->queenThreatByKnight[Us] = knightAttacksScore;
			trace->queenThreatBySlider[Us] = sliderAttacksScore;
		}
	}

	const auto safe = ~_allAttacks[Them] | _allAttacks[Us];
	const auto safePawnsAttacks =
        Attacks::pawnAttacks<Us>(board.getPieces(PAWN, Us) & safe) & nonPawnEnemies;
	const auto safePawnThreatScore = THREAT_BY_SAFE_PAWN * safePawnsAttacks.popcount();
	totalValue += safePawnThreatScore;

	const auto restrictedMovement = _allAttacks[Them] & _allAttacks[Us] & ~stronglyProtected;
	const Score restrictedMovementScore =
		RESTRICTED_PIECE_MOVEMENT * restrictedMovement.popcount();
	totalValue += restrictedMovementScore;

	if constexpr (Trace)
	{
		trace->threatBySafePawn[Us] = safePawnThreatScore;
		trace->restrictedMovement[Us] = restrictedMovementScore;
		trace->attacksTotal[Us] = totalValue;
	}

	return totalValue;
}

template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluatePawn(const Square square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr Dir ForwardDir = Us ? NORTH : SOUTH;
	constexpr Dir BehindDir = Us ? SOUTH : NORTH;

	Score value = PSQT[PAWN][square];

	const auto bb = Bitboard::fromSquare(square);
	const u8 rank = (Us ? rankOf(square) : 7u - rankOf(square)) - 1u;

	const auto adjacentFiles = Bitboard::fromAdjacentFiles(square);
	const auto opposed = board.getPieces(PAWN, Them) & Bitboard::fromRay(ForwardDir, square);
	const auto neighbours = board.getPieces(PAWN, Us) & adjacentFiles;
	const auto connected = neighbours & Bitboard::fromRank(square);
	const auto support =
		neighbours & Bitboard::fromRank(bb.shift<BehindDir>().bitScanForward());
	const bool isDoubled = bool(bb.shift<BehindDir>() & board.getPieces(PAWN, Us));

	if (support | connected)
	{
		const int connectedScore = PAWN_CONNECTED[rank] * (2 + bool(connected) - bool(opposed))
								   + 21 * support.popcount();

		value += Score(connectedScore, connectedScore * (rank - 2) / 4);
	} else if (!neighbours)
		value -= PAWN_ISOLATED;

	if (!support && isDoubled)
		value -= PAWN_DOUBLED;

	{ // Passed Pawn
		Bitboard rays = Bitboard::fromRay(ForwardDir, square);
		rays |= rays.shift<WEST>() | rays.shift<EAST>();

		const bool isPassedPawn = !bool(rays & board.getPieces(PAWN, Them));

		if (isPassedPawn)
			value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluateKnight(const Square square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr Dir BehindDir = Us ? SOUTH : NORTH;
	constexpr Bitboard OutpostRanks{
		Us == WHITE ? RANK_4 | RANK_5 | RANK_6 : RANK_5 | RANK_4 | RANK_3 };

	const auto bb = Bitboard::fromSquare(square);
	const auto attacks = Attacks::knightAttacks(square);

	Score value = PSQT[KNIGHT][square];

	{
		const auto pawns = board.getPieces(PAWN);
		const auto bitboard =
			OutpostRanks & (_pieceAttacks[Us][PAWN] | pawns.shift<BehindDir>());
		const auto targets = board.getPieces(Them) & ~pawns;

		if (bitboard & bb & ~CENTER_FILES // on a side outpost
			&& !(attacks & targets)        // no relevant attacks
			&& (!(targets & (bb & QUEEN_SIDE ? QUEEN_SIDE : KING_SIDE)).several()))
			value += UNCONTESTED_OUTPOST *
					 (pawns & (bb & QUEEN_SIDE ? QUEEN_SIDE : KING_SIDE)).popcount();
	}

	const int mobility = (attacks & _mobilityArea[Us]).popcount();
	value += KNIGHT_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	return value;
}

template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluateBishop(const Square square) const noexcept
{
	constexpr Color Them = ~Us;
	constexpr Dir Down{ Us ? SOUTH : NORTH };

	const auto bb = Bitboard::fromSquare(square);
	Score value = PSQT[BISHOP][square];

	const i32 mobility = (Attacks::bishopAttacks(square, board.occupied) &
						  _mobilityArea[Us]).popcount();
	value += BISHOP_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	// Penalty according to the number of our pawns on the same color square as the
	// bishop, bigger when the center files are blocked with pawns and smaller
	// when the bishop is outside the pawn chain.
	const auto blocked = board.getPieces(PAWN, Us) & board.occupied.shift<Down>();

	const i32 pawnsOnTheSameColorSquares =
		(board.getPieces(PAWN, Us) &
         (bool(DARK_SQUARES & bb) ? DARK_SQUARES : ~DARK_SQUARES)).popcount();
	value -=
		BISHOP_PAWNS[distanceToFileEdge(square)] *
		pawnsOnTheSameColorSquares
		* (!bool(_pieceAttacks[Us][PAWN] & bb) + (blocked & CENTER_FILES).popcount());

	// Enemy pawns x-rayed
	value -= BISHOP_XRAY_PAWNS *
			 (Attacks::bishopXRayAttacks(square) & board.getPieces(PAWN, Them)).popcount();

	// Long Diagonal Bishop
	const auto centerAttacks =
        Attacks::bishopAttacks(square, board.getPieces(PAWN, Them)) & CENTER_SQUARES;
	if (centerAttacks.several())
		value.mg += 45;

	// King Ring Threat
	if (Attacks::bishopAttacks(square, board.getPieces(PAWN)) & _kingRing[Them])
		value += KING_RING_THREAT_BISHOP;

	return value;
}

template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluateRook(const Square square) const noexcept
{
	constexpr Color Them = ~Us;

	Score value = PSQT[ROOK][square];

	const i32 mobility = (Attacks::rookAttacks(square, board.occupied) &
						  _mobilityArea[Us]).popcount();
	value += ROOK_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	const auto file = Bitboard::fromFile(square);

	if (!(board.getPieces(PAWN, Us) & file).empty())
	{
		value += ROOK_ON_FILE[!bool(board.getPieces(PAWN, Them) & file)];
	} else if (mobility <= 3)
	{
		const u8 kingFile = fileOf(board.getKingSq<Us>());

		if ((kingFile < 4) == (fileOf(square) < kingFile))
			value -= TRAPPED_ROOK * (1 + !board.canCastle<Us>());
	}

	if (Bitboard::fromFile(square) & _kingRing[Them])
		value += KING_RING_THREAT_ROOK;

	return value;
}

template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluateQueen(const Square square) const noexcept
{
	Score value = PSQT[QUEEN][square];

	const i32 mobility = (Attacks::queenAttacks(square, board.occupied) &
						  _mobilityArea[Us]).popcount();
	value += QUEEN_MOBILITY[mobility];

	if constexpr (Trace)
		trace->mobility[Us] += mobility;

	constexpr Bitboard InitialPosition{ FILE_D & (Us ? RANK_1 : RANK_8) };
	if ((InitialPosition & Bitboard::fromSquare(square)).empty())
		value.mg -= 14;

	return value;
}

/*
 * Needs to be called after both evaluatePieces<WHITE>() and evaluatePieces<BLACK>()
 */
template <bool Trace>
template <Color Us>
Score Eval<Trace>::evaluateKing() const noexcept
{
	// constexpr Color Them = ~Us;
	const u8 square = board.getKingSq<Us>();

	Score value = PSQT[KNIGHT][square];

	if (board.isCastled<Us>())
		value.mg += 70;
	else
	{
		const short count = short(board.canCastleKs<Us>()) + short(board.canCastleQs<Us>());
		value.mg += count * 20;
	}

	value += KING_PAWN_SHIELD *
			 (MASK_PAWN_SHIELD[Us][square] & board.getPieces(PAWN, Us)).popcount();

	return value;
}
