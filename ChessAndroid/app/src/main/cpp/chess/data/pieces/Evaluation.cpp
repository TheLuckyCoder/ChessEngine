#include "Evaluation.h"

#include <array>

#include "MoveGen.h"
#include "../Board.h"

constexpr int PAWN = 100;
constexpr int KNIGHT = 300;
constexpr int BISHOP = 300;
constexpr int ROOK = 500;
constexpr int QUEEN = 900;
constexpr int KING = 9000;

constexpr int PAWN_CAN_PROMOTE = 750;
constexpr int KING_IS_CASTLED = 40;

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

constexpr EvalArray KING_WHITE_ENDING =
{ {
	{-30, -25, -20, -20, -20, -20, -25, -30},
	{-25, -15, -10, -10, -10, -10, -15, -25},
	{-20, -10,   5,   5,   5,   5,   0, -20},
	{-20, -10,   5,  15,  15,   5,   0, -20},
	{-20, -10,   5,  15,  15,   5,   0, -20},
	{-20, -10,   5,   5,   5,   5,   0, -20},
	{-25, -15, -10, -10, -10, -10, -15,  25},
	{-30, -25, -20, -20, -20, -20, -25, -30}
} };

constexpr EvalArray PAWN_BLACK = reverseArray(PAWN_WHITE);
constexpr EvalArray KNIGHT_BLACK = reverseArray(KNIGHT_WHITE);
constexpr EvalArray BISHOP_BLACK = reverseArray(BISHOP_WHITE);
constexpr EvalArray ROOK_BLACK = reverseArray(ROOK_WHITE);
constexpr EvalArray QUEEN_BLACK = reverseArray(QUEEN_WHITE);
constexpr EvalArray KING_BLACK = reverseArray(KING_WHITE);
constexpr EvalArray KING_BLACK_ENDING = reverseArray(KING_WHITE_ENDING);

int Evaluation::evaluate(const Board &board)
{
	int value = 0, count = 0;

	const auto phase = determineGamePhase(board);

	for (byte i = 0; i < 8; i++)
		for (byte j = 0; j < 8; j++)
			if (const auto &piece = board.data[i][j]; piece)
			{
				value += evaluatePiece(piece, Pos(i, j), board, phase);
				if (piece.isWhite) count++; else count--;
			}

	value += count * 40;

	return value;
}

int Evaluation::evaluatePiece(const Piece &piece, const Pos &pos, const Board &board, GamePhase phase)
{
	const int points = [&]() {
		switch (piece.type)
		{
		case Piece::Type::PAWN:
			return evaluatePawn(piece, pos, board);
		case Piece::Type::KNIGHT:
			return evaluateKnight(piece, pos, board);
		case Piece::Type::BISHOP:
			return evaluateBishop(piece, pos, board);
		case Piece::Type::ROOK:
			return evaluateRook(piece, pos, board);
		case Piece::Type::QUEEN:
			return evaluateQueen(piece, pos);
		case Piece::Type::KING:
			return evaluateKing(piece, pos, board, phase);
		default:
			return 0;
		}
	}();

	return piece.isWhite ? points : -points;
}

int Evaluation::evaluatePawn(const Piece &piece, const Pos &pos, const Board &board)
{
	int value = PAWN + (piece.isWhite ? PAWN_WHITE[pos.y][pos.x] : PAWN_BLACK[pos.y][pos.x]);

	bool isolated = true;

	if (const auto &pieceAbove = board.data[pos.x][pos.y + 1];
		piece.hasSameColor(pieceAbove) && pieceAbove.type == Piece::Type::PAWN)
	{
		value -= 5;
		isolated = false;
	} else if (const auto &pieceBelow = board.data[pos.x][pos.y - 1];
		piece.hasSameColor(pieceBelow) && pieceBelow.type == Piece::Type::PAWN)
	{
		value -= 5;
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
		value -= 2;

	return value;
}

int Evaluation::evaluateKnight(const Piece &piece, const Pos &pos, const Board &board)
{
	auto moves = MoveGen::generateKnightMoves(board[pos], pos, board);

	return KNIGHT +
		(piece.isWhite ? KNIGHT_WHITE[pos.y][pos.x] : KNIGHT_BLACK[pos.y][pos.x]) +
		static_cast<int>(moves.size() / 2);
}

int Evaluation::evaluateBishop(const Piece &piece, const Pos &pos, const Board &board)
{
	auto moves = MoveGen::generateBishopMoves(board[pos], pos, board);

	return BISHOP +
		(piece.isWhite ? BISHOP_WHITE[pos.y][pos.x] : BISHOP_BLACK[pos.y][pos.x]) +
		static_cast<int>(moves.size() / 2);
}

int Evaluation::evaluateRook(const Piece &piece, const Pos &pos, const Board &board)
{
	auto moves = MoveGen::generateRookMoves(board[pos], pos, board);

	return ROOK +
		(piece.isWhite ? ROOK_WHITE[pos.y][pos.x] : ROOK_BLACK[pos.y][pos.x]) +
		static_cast<int>(moves.size() / 3);
}

int Evaluation::evaluateQueen(const Piece &piece, const Pos &pos)
{
	return QUEEN + (piece.isWhite ? QUEEN_WHITE[pos.y][pos.x] : QUEEN_BLACK[pos.y][pos.x]);
}

int Evaluation::evaluateKing(const Piece &piece, const Pos &pos, const Board &board, const GamePhase phase)
{
	int value = KING;

	if (phase == GamePhase::ENDING)
		value += piece.isWhite ? KING_WHITE_ENDING[pos.y][pos.x] : KING_BLACK_ENDING[pos.y][pos.x];
	else
	{
		value += piece.isWhite ? KING_WHITE[pos.y][pos.x] : KING_BLACK[pos.y][pos.x];
		if ((piece.isWhite && board.whiteCastled) ||
			(!piece.isWhite && board.blackCastled))
			value += KING_IS_CASTLED;

		if (!board[pos].hasBeenMoved)
			value += 8;
	}

	return value;
}

GamePhase Evaluation::determineGamePhase(const Board &board)
{
	return GamePhase::MIDDLE; // TODO
}
