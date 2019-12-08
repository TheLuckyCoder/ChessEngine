#include "Evaluation.h"

#include "MoveGen.h"
#include "../Stats.h"
#include "../data/Psqt.h"
#include "../data/Rays.h"

#define S Score

constexpr short TEMPO_BONUS = 20;
constexpr S CHECK_BONUS{ 30, 42 };

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
constexpr S PASSED_PAWN_RANK[] =
{
	S(0, 0), S(10, 28), S(17, 33), S(15, 41), S(62, 72), S(168, 177), S(276, 260)
};
constexpr S ROOK_ON_PAWN(10, 28);
constexpr S ROOK_ON_FILE[] =
{
	S(0, 0), S(21, 4), S(47, 25)
};
constexpr S ROOK_THREATS[] =
{
	S(0, 0), S(3, 44), S(38, 71), S(38, 61), S(0, 38), S(51, 38), S(0, 0)
};
constexpr S THREAT_SAFE_PAWN(173, 94);
constexpr S WEAK_QUEEN(49, 15);

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
	if (board.state == State::INVALID || board.state == State::DRAW)
		return 0;
	if (board.state == State::WINNER_WHITE)
		return VALUE_WINNER_WHITE;
	if (board.state == State::WINNER_BLACK)
		return VALUE_WINNER_BLACK;
	
	Score score[2]{};
	for (byte i = 0; i < 64u; i++)
	{
		const Piece &piece = board.getPiece(i);
		score[piece.isWhite] += Psqt::s_Bonus[piece.type][i];
	}

	const Phase phase = board.getPhase();
	const Score finalScore = score[WHITE] - score[BLACK];
	return phase == Phase::MIDDLE ? finalScore.mg : finalScore.eg;
}

short Evaluation::evaluate(const Board &board) noexcept
{
	if (board.state == State::INVALID || board.state == State::DRAW)
		return 0;
	if (board.state == State::WINNER_WHITE)
		return VALUE_WINNER_WHITE;
	if (board.state == State::WINNER_BLACK)
		return VALUE_WINNER_BLACK;

	Stats::incrementBoardsEvaluated();

	Score totalScore[2]{};
	byte pawnCount[2]{};
	byte bishopCount[2]{};
	short npm[2]{};

	const AttacksMap whiteMoves = Player::getAttacksPerColor(true, board);
	const AttacksMap blackMoves = Player::getAttacksPerColor(false, board);

	for (byte i = 0; i < 64u; i++)
		if (const auto &piece = board.getPiece(i); piece && piece.type != KING)
		{
			if (piece.type == PAWN)
			{
				pawnCount[piece.isWhite]++;
				continue;
			}

			const bool isWhite = piece.isWhite;
			const Pos pos(i);
			const auto defendedValue = isWhite ? whiteMoves.map[pos] : blackMoves.map[pos];
			const auto attackedValue = isWhite ? blackMoves.map[pos] : whiteMoves.map[pos];
			Score points;

			if (defendedValue < attackedValue)
				points -= OVERLOAD * (attackedValue - defendedValue);

			if (piece.type == BISHOP)
				bishopCount[isWhite]++;

			const auto &theirAttacks = isWhite ? whiteMoves.board : blackMoves.board;
			const U64 bb = pos.toBitboard();

			for (byte i = KNIGHT; i < BISHOP; i++)
				if (theirAttacks[!isWhite][i] & bb)
					points -= MINOR_THREATS[piece.type];

			if (theirAttacks[!isWhite][ROOK] & bb)
				points -= ROOK_THREATS[piece.type];

			npm[piece.isWhite] += getPieceValue(piece.type);
			totalScore[piece.isWhite] += points;
		}

	// 2 Bishops receive a bonus
	if (bishopCount[WHITE] >= 2)
		totalScore[WHITE] += 10;
	if (bishopCount[BLACK] >= 2)
		totalScore[BLACK] += 10;

	for (byte i = 0; i < 64u; i++)
		if (const Piece &piece = board.getPiece(i); piece)
		{
			const Score points = [&] {
				switch (piece.type)
				{
				case PAWN:
					return evaluatePawn(piece, i, board, piece.isWhite ? whiteMoves : blackMoves,
							piece.isWhite ? blackMoves : whiteMoves);
				case KNIGHT:
					return evaluateKnight(piece, i, board);
				case BISHOP:
					return evaluateBishop(piece, i, board);
				case ROOK:
					return evaluateRook(piece, i, board);
				case QUEEN:
					return evaluateQueen(piece, i, board);
				case KING:
					return evaluateKing(piece, i, board);
				default:
					return Score();
				}
			}();

			totalScore[piece.isWhite] += points;
		}

	if (board.state == State::BLACK_IN_CHECK)
		totalScore[WHITE] += CHECK_BONUS;
	else if (board.state == State::WHITE_IN_CHECK)
		totalScore[BLACK] += CHECK_BONUS;

	totalScore[board.colorToMove] += TEMPO_BONUS;

	const Score finalScore = totalScore[WHITE] - totalScore[BLACK];
	const Phase phase = board.getPhase();
	return phase == Phase::MIDDLE ? finalScore.mg : finalScore.eg;
}

Score Evaluation::evaluatePawn(const Piece &piece, const byte square, const Board &board,
							   const AttacksMap &ourAttacks, const AttacksMap &theirAttacks) noexcept
{
	const Pos pos(square);
	Score value = Psqt::s_Bonus[PAWN][square];

	const Color color = toColor(piece.isWhite);
	const byte behind = color ? -1 : 1;
	const int supported = board.at(pos.x - 1u, pos.y + behind).isSameType(piece)
						+ board.at(pos.x + 1u, pos.y + behind).isSameType(piece);

	bool isolated = !static_cast<bool>(supported);

	if (board.getPiece(pos.x, pos.y + behind).isSameType(piece))
		value -= PAWN_DOUBLED;

	if (isolated)
	{
		for (byte y = 0u ; y < 8u; y++) {
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
	if (ourAttacks.map[square] || theirAttacks.map[square] == 0) // check if the pawn is safe
	{
		const auto isEnemyPiece = [&] (const byte x, const byte y) {
			const Pos newPos(pos, x, y);
			if (!newPos.isValid()) return false;

			const Piece &other = board[newPos];
			const byte type = other.type;
			return other.isWhite != piece.isWhite && type > PAWN && type < KING;
		};

		const int i = static_cast<int>(isEnemyPiece(-1, -behind))
					+ static_cast<int>(isEnemyPiece(1u, -behind));

		value += THREAT_SAFE_PAWN * i;
	}

	const auto isPassedPawn = [&] {
		const Rays::Dir direction = color ? Rays::NORTH : Rays::SOUTH;

		const Pos posLeft(pos, -1, -behind);
		const Pos posRight(pos, 1, -behind);

		U64 rays = Rays::getRay(direction, square);
		if (posLeft.isValid())
			rays |= Rays::getRay(direction, posLeft.toSquare());
		if (posRight.isValid())
			rays |= Rays::getRay(direction, posRight.toSquare());

		return static_cast<bool>(rays & board.getType(oppositeColor(color), PAWN));
	}();

	if (isPassedPawn)
	{
		const auto rank = (piece.isWhite ? pos.y : 7u - pos.y) - 1u;
		value += PASSED_PAWN_RANK[rank];
	}

	return value;
}

inline Score Evaluation::evaluateKnight(const Piece &piece, const byte square, const Board &board) noexcept
{
	Score value = Psqt::s_Bonus[KNIGHT][square];

	const int mobility = Bitboard::popCount(MoveGen<ALL>::generateKnightMoves(piece, square, board));
	value += KNIGHT_MOBILITY[mobility];

	return value;
}

Score Evaluation::evaluateBishop(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Pos pos(square);
	Score value = Psqt::s_Bonus[BISHOP][square];

	const int mobility = Bitboard::popCount(MoveGen<ALL>::generateBishopMoves(piece, square, board));
	value += BISHOP_MOBILITY[mobility];

	// Long Diagonal Bishop
	const auto isLongDiagonalBishop = [&] {
		if (pos.y - pos.x == 0 || pos.y - (7 - pos.x) == 0)
		{
			auto [x, y] = pos;
			if (std::min<byte>(x, 7u - x) > 2) return false;
			for (byte i = std::min<byte>(x, 7u - x); i < 4; i++)
			{
				if (board.getPiece(x, y).type == PAWN) return false;
				if (x < 4) x++; else x--;
				if (y < 4) y++; else y--;
			}
		}

		return true;
	}();

	if (isLongDiagonalBishop)
		value.mg += 44;

	return value;
}

Score Evaluation::evaluateRook(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Pos pos(square);
	const Color color = toColor(piece.isWhite);
	Score value = Psqt::s_Bonus[ROOK][pos.toSquare()];

	const int mobility = Bitboard::popCount(MoveGen<ALL>::generateRookMoves(piece, pos.toSquare(), board));
	value += ROOK_MOBILITY[mobility];

	const int rookOnPawn = [&] {
		if ((piece.isWhite && pos.y < 5) || (!piece.isWhite && pos.y > 4)) return 0;

		const U64 vertical = Rays::getRay(Rays::NORTH, square) | Rays::getRay(Rays::SOUTH, square);
		const U64 horizontal = Rays::getRay(Rays::WEST, square) | Rays::getRay(Rays::EAST, square);
		const U64 rays = vertical | horizontal;

		return Bitboard::popCount(rays & board.getType(oppositeColor(color), PAWN));
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

Score Evaluation::evaluateQueen(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Pos pos(square);
	Score value = Psqt::s_Bonus[QUEEN][square];

	const int mobility = Bitboard::popCount(MoveGen<ALL>::generateQueenMoves(piece, square, board));
	value += QUEEN_MOBILITY[mobility];

	if (const U64 originalPosition = FILE_D & (piece.isWhite ? RANK_1 : RANK_8);
		(originalPosition & Bitboard::shiftedBoards[square]) == 0ull)
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
					const Piece &other = board[dPos];
					if (other.type == ROOK && (ix == 0 || iy == 0) && count == 1) return true;
					if (other.type == BISHOP && (ix != 0 && iy != 0) && count == 1) return true;
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

inline Score
Evaluation::evaluateKing(const Piece &piece, const byte square, const Board &board) noexcept
{
	const Color color = toColor(piece.isWhite);
	Score value = Psqt::s_Bonus[KNIGHT][square];

	if (board.isCastled(color))
		value.mg += 57;
	else {
		const int count = static_cast<int>(board.canCastleKs(color))
						+ static_cast<int>(board.canCastleQs(color));
		value.mg += count * 23;
	}

	return value;
}
