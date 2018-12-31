#include "PieceEval.h"

#include <array>

#include "MoveGen.h"
#include "../Board.h"

constexpr int PAWN = 100;
constexpr int KNIGHT = 300;
constexpr int BISHOP = 300;
constexpr int ROOK = 500;
constexpr int QUEEN = 900;
constexpr int KING = 9000;

using EvalArray = std::array<std::array<short, 8>, 8>;

constexpr void swapArrays(std::array<short, 8> &arr1, std::array<short, 8> &arr2)
{
	const auto copy = arr1;
	arr1 = arr2;
	arr2 = copy;
}

constexpr EvalArray reverseArray(EvalArray arr)
{
	for (std::size_t i = 0; i < arr.size(); i++)
		swapArrays(arr[i], arr[arr.size() - i - 1]);
	return arr;
}

constexpr EvalArray PAWN_WHITE
{ {
	{ 0,   0,   0,   0,   0,   0,   0,  0},
	{50,  50,  50,  50,  50,  50,  50, 50},
	{10,  10,  20,  30,  30,  20,  10, 10},
	{ 5,   5,  10,  25,  25,  10,   5,  5},
	{ 0,   0,   0,  20,  20,   0,   0,  0},
	{ 5,  -5, -10,   0,   0, -10,  -5,  5},
	{ 5,  10,  10, -20, -20,  10,  10,  5},
	{ 0,   0,   0,   0,   0,   0,   0,  0}
} };

constexpr EvalArray KNIGHT_WHITE
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

constexpr EvalArray BISHOP_WHITE
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

constexpr EvalArray ROOK_WHITE
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

constexpr EvalArray QUEEN_WHITE
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

constexpr EvalArray KING_WHITE =
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

constexpr EvalArray PAWN_BLACK = reverseArray(PAWN_WHITE);
constexpr EvalArray KNIGHT_BLACK = reverseArray(KNIGHT_WHITE);
constexpr EvalArray BISHOP_BLACK = reverseArray(BISHOP_WHITE);
constexpr EvalArray ROOK_BLACK = reverseArray(ROOK_WHITE);
constexpr EvalArray QUEEN_BLACK = reverseArray(QUEEN_WHITE);
constexpr EvalArray KING_BLACK = reverseArray(KING_WHITE);

int PieceEval::evaluatePawn(const bool isMaximizing, const Pos &pos, const Board &board)
{
	int points = PAWN + (isMaximizing ? PAWN_WHITE[pos.y][pos.x] : PAWN_BLACK[pos.y][pos.x]);

	const auto &piece = board[pos];

	bool isolated = true;

	if (const auto &other = board.data[pos.x][pos.y + 1];
		piece.hasSameColor(other) && other.type == Piece::Type::PAWN)
	{
		points -= 5;
		isolated = false;
	}

	if (const auto &other = board.data[pos.x][pos.y - 1];
		piece.hasSameColor(other) && other.type == Piece::Type::PAWN)
	{
		points -= 5;
		isolated = false;
	}

	const std::array<Pos, 6> nearby {
		pos + Pos(0, 1),
		pos + Pos(1, 1),
		pos + Pos(1, -1),
		pos + Pos(-1, -1),
		pos + Pos(0, -1),
		pos + Pos(-1, 1),
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
		points -= 2;

	return points;
}

int PieceEval::evaluateKnight(const bool isMaximizing, const Pos &pos, const Board &board)
{
	std::vector<Pos> moves;
	moves.reserve(8);
	MoveGen::generateKnightMoves(board[pos], pos, moves, board);

	return KNIGHT +
		(isMaximizing ? KNIGHT_WHITE[pos.y][pos.x] : KNIGHT_BLACK[pos.y][pos.x]) +
		static_cast<int>(moves.size() / 2);
}

int PieceEval::evaluateBishop(const bool isMaximizing, const Pos &pos, const Board &board)
{
	std::vector<Pos> moves;
	MoveGen::generateBishopMoves(board[pos], pos, moves, board);

	return BISHOP +
		(isMaximizing ? BISHOP_WHITE[pos.y][pos.x] : BISHOP_BLACK[pos.y][pos.x]) +
		static_cast<int>(moves.size() / 2);
}

int PieceEval::evaluateRook(const bool isMaximizing, const Pos &pos, const Board &board)
{
	std::vector<Pos> moves;
	MoveGen::generateRookMoves(board[pos], pos, moves, board);

	return ROOK +
		(isMaximizing ? ROOK_WHITE[pos.y][pos.x] : ROOK_BLACK[pos.y][pos.x]) +
		static_cast<int>(moves.size() / 2);
}

int PieceEval::evaluateQueen(const bool isMaximizing, const Pos &pos)
{
	return QUEEN + (isMaximizing ? QUEEN_WHITE[pos.y][pos.x] : QUEEN_BLACK[pos.y][pos.x]);
}

int PieceEval::evaluateKing(const bool isMaximizing, const Pos &pos, const Board &board)
{
	int points = KING + (isMaximizing ? KING_WHITE[pos.y][pos.x] : KING_BLACK[pos.y][pos.x]);

	if (!board[pos].hasBeenMoved)
		points += 2;

	return points;
}
