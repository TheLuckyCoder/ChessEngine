#include "Evaluation.h"

#include "MoveGen.h"
#include "../Stats.h"
#include "../data/Psqt.h"
#include "../data/Board.h"
#include "../data/Enums.h"

#define S Score

/*
 * These values were imported from the Stockfish Chess Engine
 */
constexpr S MINOR_THREATS[] =
{
	S(0, 0), S(0, 31), S(39, 42), S(57, 44), S(68, 112), S(62, 120), S(0, 0)
};
constexpr S OVERLOAD(12, 6);
constexpr S PAWN_DOUBLED(11, 56);
constexpr S PAWN_ISOLATED(5, 15);
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
	if (board.state == State::DRAW)
		return 0;
	if (board.state == State::WINNER_WHITE)
		return VALUE_WINNER_WHITE;
	if (board.state == State::WINNER_BLACK)
		return VALUE_WINNER_BLACK;
	
	short score{};
	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const Piece &piece = board.getPiece(x, y); piece)
			{
				const short points = [&]() -> short {
					switch (piece.type)
					{
						case Type::PAWN:
							return Psqt::s_PawnSquares[x][y].mg;
						case Type::KNIGHT:
							return Psqt::s_KnightSquares[x][y].mg;
						case Type::BISHOP:
							return Psqt::s_BishopSquares[x][y].mg;
						case Type::ROOK:
							return Psqt::s_RookSquares[x][y].mg;
						case Type::QUEEN:
							return Psqt::s_QueenSquares[x][y].mg;
						case Type::KING:
							return Psqt::s_KingSquares[x][y].mg;
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

	byte pawnCount[2]{};
	byte bishopCount[2]{};
	short whiteNpm = 0;
	short blackNpm = 0;

	const auto whiteMoves = MoveGen<ATTACKS_DEFENSES>::getAttacksPerColor(true, board);
	const auto blackMoves = MoveGen<ATTACKS_DEFENSES>::getAttacksPerColor(false, board);

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.getPiece(x, y); piece && piece.type != Type::KING)
			{
				if (piece.type == Type::PAWN)
				{
					pawnCount[piece.isWhite]++;
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
					bishopCount[isWhite]++;

				const auto &theirAttacks = isWhite ? whiteMoves.board : blackMoves.board;
				const U64 square = pos.toBitboard();

				for (byte i = Type::KNIGHT; i < Type::BISHOP; i++)
					if (theirAttacks[!isWhite][i] & square)
						points -= MINOR_THREATS[piece.type];

				if (theirAttacks[!isWhite][Type::ROOK] & square)
					points -= ROOK_THREATS[piece.type];

				if (isWhite) {
					whiteNpm += getPieceValue(piece.type);
					totalScore += points;
				} else {
					blackNpm += getPieceValue(piece.type);
					totalScore -= points;
				}
			}

	// 2 Bishops receive a bonus
	if (bishopCount[1] >= 2)
		totalScore += 10;
	if (bishopCount[0] >= 2)
		totalScore -= 10;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const Piece &piece = board.getPiece(x, y); piece)
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

	if (board.state == State::BLACK_IN_CHECK)
	{
		totalScore.mg += 30;
		totalScore.eg += 40;
	}
	else if (board.state == State::WHITE_IN_CHECK)
	{
		totalScore.mg -= 30;
		totalScore.eg -= 40;
	}

	// TODO: Scale Factor
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
						if (piece.type == Type::PAWN) open[!piece.isWhite] = 1;

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
	//const short p = static_cast<int>(board.getPhase());
	//return ((totalScore.mg * p + totalScore.eg * (128 - p)) / 128) + tempo;
}

Score Evaluation::evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, const Attacks &ourAttacks, const Attacks &theirAttacks) noexcept
{
	Score value = Psqt::s_PawnSquares[pos.x][pos.y];

	const byte behind = piece.isWhite ? -1 : 1;
	const int supported = (board.at(pos.x - 1u, pos.y + behind).isSameType(piece) +
		board.at(pos.x + 1u, pos.y + behind).isSameType(piece));

	bool isolated = !static_cast<bool>(supported);

	if (board.getPiece(pos.x, pos.y + behind).isSameType(piece))
		value -= PAWN_DOUBLED;

	if (isolated)
	{
		for (int y = 0 ; y < 8; y++) {
			if (board.at(pos.x - 1u, y).isSameType(piece) ||
				board.at(pos.x + 1u, y).isSameType(piece))
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
		const auto isEnemyPiece = [&] (const byte x, const byte y) {
			const Pos newPos(pos, x, y);
			if (!newPos.isValid()) return false;

			const Piece &other = board[newPos];
			const byte type = other.type;
			return other.isWhite != piece.isWhite && type > Type::PAWN && type < Type::KING;
		};

		byte i = 0;
		if (isEnemyPiece(-1, 1u)) i++;
		if (isEnemyPiece(1u, 1u)) i++;

		value += THREAT_SAFE_PAWN * i;
	}

	const auto isPassedPawn = [&] {
		const Piece enemyPawn(Type::PAWN, !piece.isWhite);

		if (piece.isWhite)
		{
			byte x = pos.x;
			for (byte y = pos.y + 1u; y < 7; y++)
				if (board.getPiece(x, y).isSameType(enemyPawn))
					return false;
			if (x > 0)
			{
				--x;
				for (byte y = pos.y + 1u; y < 7; y++)
					if (board.getPiece(x, y).isSameType(enemyPawn))
						return false;
			}

			if (x < 7)
			{
				++x;
				for (byte y = pos.y + 1u; y < 7; y++)
					if (board.getPiece(x, y).isSameType(enemyPawn))
						return false;
			}
		} else {
			byte x = pos.x;
			for (byte y = pos.y - 1u; y > 1; y--)
				if (board.getPiece(x, y).isSameType(enemyPawn))
					return false;

			if (x > 0)
			{
				--x;
				for (byte y = pos.y - 1u; y > 1; y--)
					if (board.getPiece(x, y).isSameType(enemyPawn))
						return false;
			}

			if (x < 7)
			{
				++x;
				for (byte y = pos.y - 1u; y > 1; y--)
					if (board.getPiece(x, y).isSameType(enemyPawn))
						return false;
			}
		}

		return true;
	}();

	if (isPassedPawn)
	{
		const auto rank = (piece.isWhite ? pos.y : 7u - pos.y) - 1u;
		value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

inline Score Evaluation::evaluateKnight(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = Psqt::s_KnightSquares[pos.x][pos.y];

	const int mobility = Bitboard::popCount(MoveGen<ALL, false>::generateKnightMoves(piece, pos, board));
	value += KNIGHT_MOBILITY[mobility];

	return value;
}

Score Evaluation::evaluateBishop(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = Psqt::s_BishopSquares[pos.x][pos.y];

	value += BISHOP_MOBILITY[MoveGen<ALL>::generateBishopMoves(piece, pos, board).size()];

	// Long Diagonal Bishop
	const auto isLongDiagonalBishop = [&] {
		if (pos.y - pos.x == 0 || pos.y - (7 - pos.x) == 0)
		{
			byte x = pos.x, y = pos.y;
			if (std::min<byte>(x, 7u - x) > 2) return false;
			for (byte i = std::min<byte>(x, 7u - x); i < 4; i++) {
				if (board.getPiece(x, y).type == Type::PAWN) return false;
				if (x < 4) x++; else x--;
				if (y < 4) y++; else y--;
			}
		}

		return true;
	}();

	if (isLongDiagonalBishop) value.mg += 44;

	return value;
}

Score Evaluation::evaluateRook(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = Psqt::s_RookSquares[pos.x][pos.y];

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
			if (board.getPiece(pos.x, y).isSameType(enemyPawn))
				count++;

		return count;
	}();
	value += ROOK_ON_PAWN * rookOnPawn;

	const auto rookOnFile = [&] {
		int open = 1;

		for (byte y = 0; y < 8; y++)
		{
			const auto &other = board.getPiece(pos.x, y);
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

Score Evaluation::evaluateQueen(const Piece &piece, const Pos &pos, const Board &board) noexcept
{
	Score value = Psqt::s_QueenSquares[pos.x][pos.y];

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
	Score value = Psqt::s_KingSquares[pos.x][pos.y];

	if (piece.moved)
	{
		if ((piece.isWhite && !board.whiteCastled) ||
			(!piece.isWhite && !board.blackCastled))
			value.mg -= 35;
	}

	return value;
}
