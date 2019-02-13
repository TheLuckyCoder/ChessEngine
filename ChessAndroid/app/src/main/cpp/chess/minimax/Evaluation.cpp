#include "Evaluation.h"

#include "MoveGen.h"
#include "../data/Board.h"
#include "../data/Enums.h"

using EvalArray = std::array<std::array<short, 8>, 8>;

constexpr Score PAWN(136, 208);
constexpr Score KNIGHT(782, 865);
constexpr Score BISHOP(830, 918);
constexpr Score ROOK(1289, 1378);
constexpr Score QUEEN(2529, 2687);

constexpr EvalArray reverseCopy(const EvalArray &source) noexcept
{
	const auto first = source.begin();
	auto last = source.end();
	auto dest = source;
	auto destFirst = dest.begin();

	while (first != last)
		*(destFirst++) = *(--last);

	return dest;
}

constexpr EvalArray BOARD_PAWN_WHITE
{ {
	{ 0,  0,   0,   0,   0,   0,   0,  0},
	{50, 50,  50,  50,  50,  50,  50, 50},
	{10, 10,  20,  30,  30,  20,  10, 10},
	{ 5,  5,  10,  35,  35,  10,   5,  5},
	{ 0,  0,   0,  25,  25,   0,   0,  0},
	{ 5, -5, -10,   0,   0, -10,  -5,  5},
	{ 5, 10,  10, -20, -20,  10,  10,  5},
	{ 0,  0,   0,   0,   0,   0,   0,  0}
} };

constexpr EvalArray BOARD_KNIGHT_WHITE
{ {
	{-50, -40, -30, -30, -30, -30, -40, -50},
	{-40, -20,   0,   0,   0,   0, -20, -40},
	{-30,   0,  10,  15,  15,  10,   0, -30},
	{-30,   5,  15,  20,  20,  15,   5, -30},
	{-30,   0,  15,  20,  20,  15,   0, -30},
	{-30,   5,  10,  15,  15,  10,   5, -30},
	{-40, -20,   0,   5,   5,   0, -20, -40},
	{-50, -40, -30, -30, -30, -30, -40, -50}
} };

constexpr EvalArray BOARD_BISHOP_WHITE
{ {
	{-20, -10, -10, -10, -10, -10, -10, -20},
	{-10,   0,   0,   0,   0,   0,   0, -10},
	{-10,   0,   5,  10,  10,   5,   0, -10},
	{-10,   5,   5,  10,  10,   5,   5, -10},
	{-10,   0,  10,  10,  10,  10,   0, -10},
	{-10,  10,  10,  10,  10,  10,  10, -10},
	{-10,   5,   0,   0,   0,   0,   5, -10},
	{-20, -10, -10, -10, -10, -10, -10, -20}
} };

constexpr EvalArray BOARD_ROOK_WHITE
{ {
	{ 0,  0,  0,  0,  0,  0,  0,  0},
	{ 5, 10, 10, 10, 10, 10, 10,  5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{-5,  0,  0,  0,  0,  0,  0, -5},
	{ 0,  0,  0,  5,  5,  0,  0,  0}
} };

constexpr EvalArray BOARD_QUEEN_WHITE
{ {
	{-20, -10, -10, -5, -5, -10, -10, -20},
	{-10,   0,   0,  0,  0,   0,   0, -10},
	{-10,   0,   5,  5,  5,   5,   0, -10},
	{ -5,   0,   5,  5,  5,   5,   0,  -5},
	{  0,   0,   5,  5,  5,   5,   0,  -5},
	{-10,   5,   5,  5,  5,   5,   0, -10},
	{-10,   0,   5,  0,  0,   0,   0, -10},
	{-20, -10, -10, -5, -5, -10, -10, -20}
} };

constexpr EvalArray BOARD_KING_WHITE =
{ {
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-30, -40, -40, -50, -50, -40, -40, -30},
	{-20, -30, -30, -40, -40, -30, -30, -20},
	{-10, -20, -20, -20, -20, -20, -20, -10},
	{ 20,  20,   0,   0,   0,   0,  20,  20},
	{ 20,  30,  10,   0,   0,  10,  30,  20}
} };

constexpr EvalArray BOARD_KING_WHITE_ENDING =
{ {
	{-60, -50, -40, -30, -30, -40, -50, -60},
	{-40, -30, -15,  -5,  -5, -15, -30, -40},
	{-40, -15,  20,  30,  30,  20, -15, -40},
	{-40, -15,  20,  30,  30,  20, -15, -40},
	{-40, -15,  30,  40,  40,  30, -15, -40},
	{-40, -15,  20,  30,  30,  20, -15, -40},
	{-40, -30,  -5,  -5,  -5,  -5, -30, -40},
	{-60, -40, -40, -40, -40, -40, -40, -60}
} };

constexpr EvalArray BOARD_PAWN_BLACK = reverseCopy(BOARD_PAWN_WHITE);
constexpr EvalArray BOARD_KNIGHT_BLACK = reverseCopy(BOARD_KNIGHT_WHITE);
constexpr EvalArray BOARD_BISHOP_BLACK = reverseCopy(BOARD_BISHOP_WHITE);
constexpr EvalArray BOARD_ROOK_BLACK = reverseCopy(BOARD_ROOK_WHITE);
constexpr EvalArray BOARD_QUEEN_BLACK = reverseCopy(BOARD_QUEEN_WHITE);
constexpr EvalArray BOARD_KING_BLACK = reverseCopy(BOARD_KING_WHITE);
constexpr EvalArray BOARD_KING_BLACK_ENDING = reverseCopy(BOARD_KING_WHITE_ENDING);

#define S Score

constexpr S PAWN_DOUBLED(11, 56);
constexpr S PAWN_ISOLATED(5, 15);

constexpr Score KNIGHT_MOBILITY[] =
{
    S(-62, -81), S(-53, -56), S(-12, -30), S(-4, -14), S(3, 8), S(13, 15),
    S(22, 23), S(28, 27), S(33, 33)
};
constexpr Score BISHOP_MOBILITY[] =
{
	S(-48,-59), S(-20,-23), S(16, -3), S(26, 13), S(38, 24), S(51, 42),
	S(55, 54), S(63, 57), S(63, 65), S(68, 73), S(81, 78), S(81, 86),
	S(91, 88), S(98, 97)
};
constexpr Score ROOK_MOBILITY[] =
{
    S(-58, -76), S(-27, -18), S(-15, 28), S(-10, 55), S(-5, 69), S(-2, 82),
    S(9, 112), S(16, 118), S(30, 132), S(29, 142), S(32, 155), S(38, 165),
    S(46, 166), S(48, 169), S(58, 171)
};
constexpr Score QUEEN_MOBILITY[] =
{
    S(-39, -36), S(-21, -15), S(3, 8), S(3, 18), S(14, 34), S(22, 54),
    S(28, 61), S(41, 73), S(43, 79), S(48, 92), S(56, 94), S(60, 104),
    S(60, 113), S(66, 120), S(67, 123), S(70, 126), S(71, 133), S(73, 136),
    S(79, 140), S(88, 143), S(88, 148), S(99, 166), S(102, 170), S(102, 175),
    S(106, 184), S(109, 191), S(113, 206), S(116, 212)
};

#undef S

int Evaluation::evaluate(const Board &board)
{
	++BoardManager::boardsEvaluated;

	Score score;
	int npm = 0;

	std::pair<short, short> bishopCount;

	const auto whiteMoves = MoveGen<ATTACKS_DEFENSES>::getAttacksPerColorMap(true, board);
	const auto blackMoves = MoveGen<ATTACKS_DEFENSES>::getAttacksPerColorMap(false, board);

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece && piece.type != Piece::Type::KING)
			{
				npm += [&]() -> short {
					switch (piece.type)
					{
					case Piece::Type::PAWN:
					case Piece::Type::KING:
					case Piece::Type::NONE:
						return 0;
					case Piece::Type::KNIGHT:
						return KNIGHT.mg;
					case Piece::Type::BISHOP:
					{
						if (piece.isWhite) bishopCount.first++; else bishopCount.second++;
						return BISHOP.mg;
					}
					case Piece::Type::ROOK:
						return ROOK.mg;
					case Piece::Type::QUEEN:
						return QUEEN.mg;
					}
				}();

				const Pos pos(x, y);
				const auto defendedValue = piece.isWhite ? whiteMoves[pos] : blackMoves[pos];
				const auto attackedValue = piece.isWhite ? blackMoves[pos] : whiteMoves[pos];

				if (defendedValue < attackedValue)
					score -= (attackedValue - defendedValue) * 10;
			}

	// 2 Bishops receive a bonus
	if (bishopCount.first >= 2)
		score += 10;
	if (bishopCount.second >= 2)
		score -= 10;

	const static int midgameLimit = 15258, endgameLimit = 3915;
	npm = std::max(endgameLimit, std::min(npm, midgameLimit));
	const auto phase = static_cast<Phase>(((npm - endgameLimit) * 128) / (midgameLimit - endgameLimit));

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece)
			{
				const Score points = [&] {
					auto &opponentsAttacks = piece.isWhite ? blackMoves : whiteMoves;
					const Pos pos(x, y);
					switch (piece.type)
					{
					case Piece::Type::PAWN:
						return evaluatePawn(piece, pos, board, opponentsAttacks);
					case Piece::Type::KNIGHT:
						return evaluateKnight(piece, pos, board);
					case Piece::Type::BISHOP:
						return evaluateBishop(piece, pos, board);
					case Piece::Type::ROOK:
						return evaluateRook(piece, pos, board);
					case Piece::Type::QUEEN:
						return evaluateQueen(piece, pos, board);
					case Piece::Type::KING:
						return evaluateKing(piece, pos, board);
					default:
						return Score();
					}
				}();

				if (piece.isWhite)
					score += points;
				else
					score -= points;
			}

	if (board.whiteCastled)
		score.mg += 20;
	if (board.blackCastled)
		score.mg -= 20;

	if (board.state == State::BLACK_IN_CHESS)
	{
		score.mg += 30;
		score.eg += 40;
	}
	else if (board.state == State::WHITE_IN_CHESS)
	{
		score.mg -= 30;
		score.eg -= 40;
	}

	if (phase == Phase::MIDDLE)
		return score.mg;
	return score.eg;
}

inline Score Evaluation::evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, const PosMap &opponentsAttacks)
{
	Score value = PAWN;
	value.mg += piece.isWhite ? BOARD_PAWN_WHITE[pos.y][pos.x] : BOARD_PAWN_BLACK[pos.y][pos.x];

	// Rook Pawns are worth 15% less because they can only attack one way
	if (pos.x == 0 || pos.x == 7)
		value -= 15;

	bool isolated = true;

	if (const auto &pieceAbove = board.data[pos.x][pos.y + 1];
		piece.hasSameColor(pieceAbove) && pieceAbove.type == Piece::Type::PAWN)
	{
		value -= PAWN_DOUBLED;
		isolated = false;
	} else if (const auto &pieceBelow = board.data[pos.x][pos.y - 1];
		piece.hasSameColor(pieceBelow) && pieceBelow.type == Piece::Type::PAWN)
	{
		value -= PAWN_DOUBLED;
		isolated = false;
	}

	const Pos nearby[6] {
		Pos(pos, 0, 1),
		Pos(pos, 1, 1),
		Pos(pos, 1, -1),
		Pos(pos, -1, -1),
		Pos(pos, 0, -1),
		Pos(pos, -1, 1)
	};

	if (isolated)
	{
		for (const auto &p : nearby)
		{
			if (p.isValid() && board[p].type == Piece::Type::PAWN)
			{
				isolated = false;
				break;
			}
		}
	}

	if (isolated)
		value -= PAWN_ISOLATED;

	if (((piece.isWhite && pos.y == 6) || (!piece.isWhite && pos.y == 1)) && opponentsAttacks[pos] == 0)
		value += 100;

	return value;
}

inline Score Evaluation::evaluateKnight(const Piece &piece, const Pos &pos, const Board &board)
{
	Score value = KNIGHT;
	value.mg += piece.isWhite ? BOARD_KNIGHT_WHITE[pos.y][pos.x] : BOARD_KNIGHT_BLACK[pos.y][pos.x];

	value += KNIGHT_MOBILITY[MoveGen<ALL>::generateKnightMoves(piece, pos, board).size()];

	return value;
}

inline Score Evaluation::evaluateBishop(const Piece &piece, const Pos &pos, const Board &board)
{
	Score value = BISHOP;

	value.mg += piece.isWhite ? BOARD_BISHOP_WHITE[pos.y][pos.x] : BOARD_BISHOP_BLACK[pos.y][pos.x];

	value += BISHOP_MOBILITY[MoveGen<ALL>::generateBishopMoves(piece, pos, board).size()];

	return value;
}

inline Score Evaluation::evaluateRook(const Piece &piece, const Pos &pos, const Board &board)
{
	Score value = ROOK;

	value.mg += piece.isWhite ? BOARD_ROOK_WHITE[pos.y][pos.x] : BOARD_ROOK_BLACK[pos.y][pos.x];

	value += ROOK_MOBILITY[MoveGen<ALL>::generateRookMoves(piece, pos, board).size()];

	if (piece.moved)
		if ((piece.isWhite && !board.whiteCastled) ||
			(!piece.isWhite && !board.blackCastled))
			value.mg -= 10;

	return value;
}

inline Score Evaluation::evaluateQueen(const Piece &piece, const Pos &pos, const Board &board)
{
	Score value = QUEEN;
	value.mg += piece.isWhite ? BOARD_QUEEN_WHITE[pos.y][pos.x] : BOARD_QUEEN_BLACK[pos.y][pos.x];

	value += QUEEN_MOBILITY[MoveGen<ALL>::generateQueenMoves(piece, pos, board).size()];

	if (piece.moved)
		value.mg -= 30;

	return value;
}

inline Score Evaluation::evaluateKing(const Piece &piece, const Pos &pos, const Board &board)
{
	Score value(piece.isWhite ? BOARD_KING_WHITE[pos.y][pos.x] : BOARD_KING_BLACK[pos.y][pos.x],
		piece.isWhite ? BOARD_KING_WHITE_ENDING[pos.y][pos.x] : BOARD_KING_BLACK_ENDING[pos.y][pos.x]);

	if (piece.moved)
	{
		if ((piece.isWhite && !board.whiteCastled) ||
			(!piece.isWhite && !board.blackCastled))
			value.mg -= 20;
	}

	return value;
}
