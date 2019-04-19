#include "Evaluation.h"

#include "MoveGen.h"
#include "../data/Board.h"
#include "../data/Enums.h"
#include "../Stats.h"

#define S Score

constexpr S PAWN_SCORE(136, 208);
constexpr S KNIGHT_SCORE(782, 865);
constexpr S BISHOP_SCORE(830, 918);
constexpr S ROOK_SCORE(1289, 1378);
constexpr S QUEEN_SCORE(2529, 2687);

constexpr S MINOR_THREATS[] =
{
	S(0, 0), S(0, 31), S(39, 42), S(57, 44), S(68, 112), S(62, 120), S(0, 0)
};
constexpr S OVERLOAD(12, 6);
constexpr S PAWN_DOUBLED(11, 56);
constexpr S PAWN_ISOLATED(5, 15);
constexpr S PASSED_PAWN_FILE[] =
{
	S(0, 0), S(-1, 7), S(0, 9), S(-9, -8), S(-30, -14)
};
constexpr S PASSED_PAWN_RANK[] =
{
	S(0, 0), S(5, 18), S(12, 23), S(10, 31), S(57, 62), S(163, 167), S(271, 250)
};
constexpr S ROOK_ON_PAWN(10, 28);
constexpr S ROOK_ON_FILE[] =
{
	S(0, 0), S(18, 7), S(44, 20)
};
constexpr S ROOK_THREATS[] =
{
	S(0, 0), S(0, 24), S(38, 71), S(38, 61), S(0, 38), S(51, 38), S(0, 0)
};
constexpr S THREAT_SAFE_PAWN(169, 99);
constexpr S WEAK_QUEEN(51, 10);

constexpr S PAWN_SQUARE[][4] =
{
	{ S(0,   0),  S(0,   0),   S(0,   0),  S(0,  0)  },
	{ S(-11, -3), S(7,   -1),  S(7,   7),  S(17, 2)  },
	{ S(-16, -2), S(-3,  2),   S(23,  6),  S(23, -1) },
	{ S(-14, 7),  S(-7,  -4),  S(20,  -8), S(24, 2)  },
	{ S(-5,  13), S(-2,  10),  S(-1,  -1), S(12, -8) },
	{ S(-11, 16), S(-12, 6),   S(-2,  1),  S(4,  16) },
	{ S(-2,  1),  S(20,  -12), S(-10, 6),  S(-2, 25) },
	{ S(0,   0),  S(0,   0),   S(0,   0),  S(0,  0)  }
};
constexpr S KNIGHT_SQUARE[][4] =
{
	{ S(-169, -105), S(-96, -74), S(-80, -46), S(-79, -18) },
	{ S(-79,  -70),  S(-39, -56), S(-24, -15), S(-9,  6)   },
	{ S(-64,  -38),  S(-20, -33), S(4,   -5),  S(19,  27)  },
	{ S(-28,  -36),  S(5,   0),   S(41,  13),  S(47,  34)  },
	{ S(-29,  -41),  S(13,  -20), S(42,  4),   S(52,  35)  },
	{ S(-11,  -51),  S(28,  -38), S(63,  -17), S(55,  19)  },
	{ S(-67,  -64),  S(-21, -45), S(6,   -37), S(37,  16)  },
	{ S(-200, -98),  S(-80, -89), S(-53, -53), S(-32, -16) }
};
constexpr S BISHOP_SQUARE[][4] =
{
	{ S(-44, -63), S(-4,  -30), S(-11, -35), S(-28, -8)  },
	{ S(-18, -38), S(7,   -13), S(14,  -14), S(3,   0)   },
	{ S(-8,  -18), S(24,  0),   S(-3,  -7),  S(15,  13)  },
	{ S(1,   -26), S(8,   -3),  S(26,  1),   S(37,  16)  },
	{ S(-7,  -24), S(30,  -6),  S(23,  -10), S(28,  17)  },
	{ S(-17, -26), S(4,   2),   S(-1,  1),   S(8,   16)  },
	{ S(-21, -34), S(-19, -18), S(10,  -7),  S(-6,  9)   },
	{ S(-48, -51), S(-3,  -40), S(-12, -39), S(-25, -20) }
};
constexpr S ROOK_SQUARE[][4] =
{
	{ S(-24, -2),  S(-13, -6), S(-7, -3),  S(2,-2)  },
	{ S(-18, -10), S(-10, -7), S(-5, 1),   S(9, 0)  },
	{ S(-21, 10),  S(-7,  -4), S(3,  2),   S(-1,-2) },
	{ S(-13, -5),  S(-5,  2),  S(-4, -8),  S(-6, 8) },
	{ S(-24, -8),  S(-12, 5),  S(-1, 4),   S(6, -9) },
	{ S(-24, 3),   S(-4,  -2), S(4,  -10), S(10, 7) },
	{ S(-8,  1),   S(6,   2),  S(10, 17),  S(12,-8) },
	{ S(-22, 12),  S(-24, -6), S(-6, 13),  S(4, 7)  }
};
constexpr S QUEEN_SQUARE[][4] =
{
	{ S(3,  -69), S(-5, -57), S(-5, -47), S(4,  -26) },
	{ S(-3, -55), S(5,  -31), S(8,  -22), S(12, -4)  },
	{ S(-3, -39), S(6,  -18), S(13, -9),  S(7,  3)   },
	{ S(4,  -23), S(5,  -3),  S(9,  13),  S(8,  24)  },
	{ S(0,  -29), S(14, -6),  S(12, 9),   S(5,  21)  },
	{ S(-4, -38), S(10, -18), S(6,  -12), S(8,  1)   },
	{ S(-5, -50), S(6,  -27), S(10, -24), S(8,  -8)  },
	{ S(-2, -75), S(-2, -52), S(1,  -43), S(-2, -36) }
};
constexpr S KING_SQUARE[][4] =
{
	{ S(272, 0),   S(325, 41),  S(273, 80),  S(190, 93)  },
	{ S(277, 57),  S(305, 98),  S(241, 138), S(183, 131) },
	{ S(198, 86),  S(253, 138), S(168, 165), S(120, 173) },
	{ S(169, 103), S(191, 152), S(136, 168), S(108, 169) },
	{ S(145, 98),  S(176, 166), S(112, 197), S(69,  194) },
	{ S(122, 87),  S(159, 164), S(85,  174), S(36,  189) },
	{ S(87,  40),  S(120, 99),  S(64,  128), S(25,  141) },
	{ S(64,  5),   S(87,  60),  S(49,  75),  S(0,   75)  }
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

short Evaluation::simpleEvaluation(const Board &board) noexcept
{
	short score = 0;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece)
			{
				const int points = [&]() -> short {
					switch (piece.type)
					{
						case Type::PAWN:
							return PAWN_SCORE.mg + PAWN_SQUARE[7u - x][std::min<byte>(y, 7u - y)].mg;
						case Type::KNIGHT:
							return KNIGHT_SCORE.mg + KNIGHT_SQUARE[7u - x][std::min<byte>(y, 7u - y)].mg;
						case Type::BISHOP:
							return BISHOP_SCORE.mg + BISHOP_SQUARE[7u - x][std::min<byte>(y, 7u - y)].mg;
						case Type::ROOK:
							return ROOK_SCORE.mg + ROOK_SQUARE[7u - x][std::min<byte>(y, 7u - y)].mg;
						case Type::QUEEN:
							return QUEEN_SCORE.mg + QUEEN_SQUARE[7u - x][std::min<byte>(y, 7u - y)].mg;
						case Type::KING:
							return KING_SQUARE[7u - x][std::min<byte>(y, 7u - y)].mg;
						default:
							return 0;
					}
				}();

				if (piece.isWhite) score += points; else score -= points;
			}

	return score;
}

short Evaluation::evaluate(const Board &board) noexcept
{
	if (board.state == State::DRAW)
		return 0;
	if (board.state == State::WINNER_WHITE)
		return VALUE_WINNER_WHITE;
	if (board.state == State::WINNER_BLACK)
		return VALUE_WINNER_BLACK;

	if (Stats::enabled())
		++Stats::boardsEvaluated;

	Score totalScore;

	PairColor<byte> pawnCount{};
	PairColor<byte> bishopCount{};
	PairColor<short> npm{};

	const auto whiteMoves = MoveGen<ATTACKS_DEFENSES>::getAttacksPerColor(true, board);
	const auto blackMoves = MoveGen<ATTACKS_DEFENSES>::getAttacksPerColor(false, board);

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece && piece.type != Type::KING)
			{
				if (piece.type == Type::PAWN)
				{
					piece.isWhite ? pawnCount.white++ : pawnCount.black++;
					continue;
				}

				const bool isWhite = piece.isWhite;
				const Pos pos(x, y);
				const auto defendedValue = isWhite ? whiteMoves.map[pos] : blackMoves.map[pos];
				const auto attackedValue = isWhite ? blackMoves.map[pos] : whiteMoves.map[pos];
				Score points;

				if (defendedValue < attackedValue)
					points -= OVERLOAD * (attackedValue - defendedValue);

				if (piece.type == Type::BISHOP)
				{
					if (isWhite) bishopCount.white++; else bishopCount.black++;
				}

				const auto &theirAttacks = isWhite ? whiteMoves.board : blackMoves.board;
				const Bitboard square = pos.toBitboard();

				for (byte i = Type::KNIGHT; i < Type::BISHOP; i++)
					if (theirAttacks[!isWhite][i] & square)
						points -= MINOR_THREATS[piece.type];

				if (theirAttacks[!isWhite][Type::ROOK] & square)
					points -= ROOK_THREATS[piece.type];

				if (isWhite) {
					npm.white += getPieceValue(piece.type);
					totalScore += points;
				} else {
					npm.black += getPieceValue(piece.type);
					totalScore -= points;
				}
			}

	// 2 Bishops receive a bonus
	if (bishopCount.white >= 2)
		totalScore += 10;
	if (bishopCount.black >= 2)
		totalScore -= 10;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece)
			{
				const Score points = [&] {
					const Pos pos(x, y);
					switch (piece.type)
					{
					case Type::PAWN:
						return evaluatePawn(piece, pos, board, piece.isWhite ? whiteMoves : blackMoves,
								piece.isWhite ? blackMoves : whiteMoves);
					case Type::KNIGHT:
						return evaluateKnight(piece, pos, board);
					case Type::BISHOP:
						return evaluateBishop(piece, pos, board);
					case Type::ROOK:
						return evaluateRook(piece, pos, board);
					case Type::QUEEN:
						return evaluateQueen(piece, pos, board);
					case Type::KING:
						return evaluateKing(piece, pos, board);
					default:
						return Score();
					}
				}();

				if (piece.isWhite) totalScore += points; else totalScore -= points;
			}

	if (board.whiteCastled)
		totalScore.mg += 60;
	if (board.blackCastled)
		totalScore.mg -= 60;

	if (board.state == State::BLACK_IN_CHESS)
	{
		totalScore.mg += 30;
		totalScore.eg += 40;
	}
	else if (board.state == State::WHITE_IN_CHESS)
	{
		totalScore.mg -= 30;
		totalScore.eg -= 40;
	}

	// Scale Factor
	/*int sf = 64;
	{
		if (pawnCount.white == 0 && npm.white - npm.black <= BISHOP_SCORE.mg) sf = npm.white < ROOK_SCORE.mg ? 0 : (npm.black <= BISHOP_SCORE.mg ? 4 : 14);
		if (sf == 64)
		{
			const bool ob = true;// = opposite_bishops(pos);
			if (ob && npm.white == BISHOP_SCORE.mg && npm.black == BISHOP_SCORE.mg) {
				int asymmetry = 0;
				for (const auto &x : board.data)
				{
					int open[]{ 0, 0 };

					for (const Piece &piece : x)
					{
						if (piece.type == Type::PAWN) open[!piece.isWhite] = 1;
					}

					if (open[0] + open[1] == 1) asymmetry++;
				}
				//asymmetry += candidate_passed_w + candidate_passed_b;
				sf = 8 + 4 * asymmetry;
			} else {
				sf = std::min(40 + (ob ? 2 : 7) * pawnCount.white, sf);
			}
		}
	}
	totalScore.eg *= sf / 64;*/

	// Tempo
	const int tempo = board.whiteToMove ? 20 : -20;
	totalScore += tempo;
	const Phase phase = board.getPhase();

	return phase == Phase::MIDDLE ? totalScore.mg : totalScore.eg;
	//const int p = static_cast<int>(board.getPhase());
	//return ((totalScore.mg * p + totalScore.eg * (128 - p)) / 128) + tempo;
}

Score Evaluation::evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, const Attacks &ourAttacks, const Attacks &theirAttacks) noexcept
{
	Score value = PAWN_SCORE;
	value += PAWN_SQUARE[7u - pos.x][std::min<byte>(pos.y, 7u - pos.y)];

	const byte behind = piece.isWhite ? -1 : 1;
	const int supported = (piece.isSameType(board.getPieceSafely(pos.x - 1u, pos.y + behind)) +
							piece.isSameType(board.getPieceSafely(pos.x + 1u, pos.y + behind)));

	bool isolated = !static_cast<bool>(supported);

	if (piece.isSameType(board.data[pos.x][pos.y + behind]))
		value -= PAWN_DOUBLED;

	if (isolated)
	{
		for (int y = 0 ; y < 8; y++) {
			if (board.getPieceSafely(pos.x - 1u, y).isSameType(piece) ||
				board.getPieceSafely(pos.x + 1u, y).isSameType(piece))
			{
				isolated = false;
				break;
			}
		}
	}

	if (isolated)
		value -= PAWN_ISOLATED;

	// Threat Safe Pawn
	if (ourAttacks.map[pos] || theirAttacks.map[pos] == 0) // check if the pawn is safe
	{
		const auto isEnemyPiece = [&] (const Pos &newPos) {
			if (!newPos.isValid()) return false;

			const Piece &other = board[newPos];
			const byte type = other.type;
			return other.isWhite != piece.isWhite && type > Type::PAWN && type < Type::KING;
		};

		byte i = 0;
		if (isEnemyPiece(Pos(pos, -1, 1))) i++;
		if (isEnemyPiece(Pos(pos, 1, 1 ))) i++;

		value += THREAT_SAFE_PAWN * i;
	}

	// TODO: Check for passed pawns

	return value;
}

inline Score Evaluation::evaluateKnight(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = KNIGHT_SCORE;

	value += KNIGHT_SQUARE[7u - pos.x][std::min<byte>(pos.y, 7u - pos.y)];
	value += KNIGHT_MOBILITY[MoveGen<ALL>::generateKnightMoves(piece, pos, board).size()];

	return value;
}

inline Score Evaluation::evaluateBishop(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = BISHOP_SCORE;

	value += BISHOP_SQUARE[7u - pos.x][std::min<byte>(pos.y, 7u - pos.y)];
	value += BISHOP_MOBILITY[MoveGen<ALL>::generateBishopMoves(piece, pos, board).size()];

	// Long Diagonal Bishop
	if (pos.y - pos.x == 0 || pos.y - (7 - pos.x) == 0)
	{
		byte x = pos.x, y = pos.y;
		if (std::min<byte>(x, 7u - x) > 2) return value;
		for (byte i = std::min<byte>(x, 7u - x); i < 4; i++) {
			if (board.data[x][y].type == Type::PAWN) return value;
			if (x < 4) x++; else x--;
			if (y < 4) y++; else y--;
		}
	}

	value.mg += 44;

	return value;
}

inline Score Evaluation::evaluateRook(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = ROOK_SCORE;

	value += ROOK_SQUARE[7u - pos.x][std::min<byte>(pos.y, 7u - pos.y)];
	value += ROOK_MOBILITY[MoveGen<ALL>::generateRookMoves(piece, pos, board).size()];

	if (piece.moved)
	{
		if ((piece.isWhite && !board.whiteCastled) ||
			(!piece.isWhite && !board.blackCastled))
			value.mg -= 10;
	}

	const int rookOnPawn = [&] {
		if ((piece.isWhite && pos.y < 5) || (!piece.isWhite && pos.y > 4)) return 0;

		const Piece enemyPawn(PAWN, !piece.isWhite);
		int count = 0;

		for (const auto &x : board.data)
			if (x[pos.y].isSameType(enemyPawn))
				count++;
		for (byte y = 0; y < 8; y++)
			if (board.data[pos.x][y].isSameType(enemyPawn))
				count++;

		return count;
	}();
	value += ROOK_ON_PAWN * rookOnPawn;

	const auto rookOnFile = [&] {
		int open = 1;

		for (byte y = 0; y < 8; y++)
		{
			const auto &other = board.data[pos.x][y];
			if (other.type == PAWN)
			{
				if (piece.isSameColor(other))
					return 0;
				open = 0;
			}
		}

		return open + 1;
	}();
	value += ROOK_ON_FILE[rookOnFile];

	return value;
}

inline Score Evaluation::evaluateQueen(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = QUEEN_SCORE;

	value += QUEEN_SQUARE[7u - pos.x][std::min<byte>(pos.y, 7u - pos.y)];
	value += QUEEN_MOBILITY[MoveGen<ALL>::generateQueenMoves(piece, pos, board).size()];

	if (piece.moved)
		value.mg -= 30;

	const auto weak = [&] {
		for (byte i = 0; i < 8; i++) {
			const byte ix = (i + (i > 3u)) % 3u - 1u;
			const byte iy = (i + (i > 3u)) / 3u - 1u;
			byte count = 0;
			for (byte d = 1; d < 8; d++) {
				Pos dPos(pos, d * ix, d * iy);
				if (dPos.isValid())
				{
					const auto &other = board[dPos];
					if (other.type == Type::ROOK && (ix == 0 || iy == 0) && count == 1) return true;
					if (other.type == Type::BISHOP && (ix != 0 && iy != 0) && count == 1) return true;
					if (other) count++;
				}
				else
					count++;
			}
		}
		return false;
	}();

	value -= WEAK_QUEEN * weak;

	return value;
}

inline Score Evaluation::evaluateKing(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = KING_SQUARE[7u - pos.x][std::min<byte>(pos.y, 7u - pos.y)];

	if (piece.moved)
	{
		if ((piece.isWhite && !board.whiteCastled) ||
			(!piece.isWhite && !board.blackCastled))
			value.mg -= 35;
	}

	return value;
}
