#pragma once

#include <array>

namespace PieceEval {

	using EvalArray = std::array<std::array<int, 8>, 8>;

	constexpr static void swapArrays(std::array<int, 8> &arr1, std::array<int, 8> &arr2)
	{
		const auto copy = arr1;
		arr1 = arr2;
		arr2 = copy;
	}

	constexpr static EvalArray reverseArray(EvalArray arr)
	{
		for (std::size_t i = 0; i < arr.size(); i++)
			swapArrays(arr[i], arr[arr.size() - i - 1]);
		return arr;
	}

	constexpr int PAWN = 100;
	constexpr int KNIGHT = 300;
	constexpr int BISHOP = 300;
	constexpr int ROOK = 500;
	constexpr int QUEEN = 900;
	constexpr int KING = 9000;

	constexpr EvalArray PAWN_WHITE
	{ {
		{ 0,   0,   0,   0,   0,   0,   0,  0},
		{50,  50,  50,  50,  50,  50,  50, 50},
		{10,  10,  20,  30,  30,  20,  10, 10},
		{ 5,   5,  10,  25,  25,  10,   5,  5},
		{ 0,   0,   0,  20,  20,   0,   0,  0},
		{ 5,  -5, -10,   0,   0, -10, - 5,  5},
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
}
