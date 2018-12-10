#pragma once

#include <array>

namespace PieceEval {

	using EvalArray = std::array<std::array<float, 8>, 8>;

	constexpr static void swapArrays(std::array<float, 8> &arr1, std::array<float, 8> &arr2)
	{
		auto copy = arr1;
		arr1 = arr2;
		arr2 = copy;
	}

	constexpr static EvalArray reverseArray(EvalArray arr)
	{
		for (std::size_t i = 0; i < arr.size(); i++)
			swapArrays(arr[i], arr[arr.size() - i - 1]);
		return arr;
	}

	constexpr float PAWN = 10.0f;
	constexpr float KNIGHT = 30.0f;
	constexpr float BISHOP = 30.0f;
	constexpr float ROOK = 50.0f;
	constexpr float QUEEN = 90.0f;
	constexpr float KING = 900.0f;

	constexpr EvalArray PAWN_WHITE
	{ {
		{0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f},
		{5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  5.0f, 5.0f},
		{1.0f,  1.0f,  2.0f,  3.0f,  3.0f,  2.0f,  1.0f, 1.0f},
		{0.5f,  0.5f,  1.0f,  2.5f,  2.5f,  1.0f,  0.5f, 0.5f},
		{0.0f,  0.0f,  0.0f,  2.0f,  2.0f,  0.0f,  0.0f, 0.0f},
		{0.5f, -0.5f, -1.0f,  0.0f,  0.0f, -1.0f, -0.5f, 0.5f},
		{0.5f,  1.0f,  1.0f, -2.0f, -2.0f,  1.0f,  1.0f, 0.5f},
		{0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 0.0f}
	} };

	constexpr EvalArray KNIGHT_WHITE
	{ {
		{-5.0f, -4.0f, -3.0f, -3.0f, -3.0f, -3.0f, -4.0f, -5.0f},
		{-4.0f, -2.0f,  0.0f,  0.0f,  0.0f,  0.0f, -2.0f, -4.0f},
		{-3.0f,  0.0f,  1.0f,  1.5f,  1.5f,  1.0f,  0.0f, -3.0f},
		{-3.0f,  0.5f,  1.5f,  2.0f,  2.0f,  1.5f,  0.5f, -3.0f},
		{-3.0f,  0.0f,  1.5f,  2.0f,  2.0f,  1.5f,  0.0f, -3.0f},
		{-3.0f,  0.5f,  1.0f,  1.5f,  1.5f,  1.0f,  0.5f, -3.0f},
		{-4.0f, -2.0f,  0.0f,  0.5f,  0.5f,  0.0f, -2.0f, -4.0f},
		{-5.0f, -4.0f, -3.0f, -3.0f, -3.0f, -3.0f, -4.0f, -5.0f}
	} };

	constexpr EvalArray BISHOP_WHITE
	{ {
		{-2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f},
		{-1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f},
		{-1.0f,  0.0f,  0.5f,  1.0f,  1.0f,  0.5f,  0.0f, -1.0f},
		{-1.0f,  0.5f,  0.5f,  1.0f,  1.0f,  0.5f,  0.5f, -1.0f},
		{-1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f},
		{-1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f},
		{-1.0f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f,  0.5f, -1.0f},
		{-2.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -2.0f}
	} };

	constexpr EvalArray ROOK_WHITE
	{ {
		{0.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  0.0f},
		{0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.5f},
		{-0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f},
		{-0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f},
		{-0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f},
		{-0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f},
		{-0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f},
		{0.0f,  0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 0.0f,  0.0f}
	} };

	constexpr EvalArray QUEEN_WHITE
	{ {
		{-2.0f, -1.0f, -1.0f, -0.5f, -0.5f, -1.0f, -1.0f, -2.0f},
		{-1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f},
		{-1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -1.0f},
		{-0.5f,  0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -0.5f},
		{0.0f,   0.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -0.5f},
		{-1.0f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.0f, -1.0f},
		{-1.0f,  0.0f,  0.5f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f},
		{-2.0f, -1.0f, -1.0f, -0.5f, -0.5f, -1.0f, -1.0f, -2.0f}
	} };

	constexpr EvalArray KING_WHITE =
	{ {
		{-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f},
		{-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f},
		{-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f},
		{-3.0f, -4.0f, -4.0f, -5.0f, -5.0f, -4.0f, -4.0f, -3.0f},
		{-2.0f, -3.0f, -3.0f, -4.0f, -4.0f, -3.0f, -3.0f, -2.0f},
		{-1.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -2.0f, -1.0f},
		{2.0f,   2.0f,  0.0f,  0.0f,  0.0f,  0.0f,  2.0f,  2.0f},
		{2.0f,   3.0f,  1.0f,  0.0f,  0.0f,  1.0f,  3.0f,  2.0f}
	} };

	constexpr EvalArray PAWN_BLACK = reverseArray(PAWN_WHITE);
	constexpr EvalArray KNIGHT_BLACK = reverseArray(KNIGHT_WHITE);
	constexpr EvalArray BISHOP_BLACK = reverseArray(BISHOP_WHITE);
	constexpr EvalArray ROOK_BLACK = reverseArray(ROOK_WHITE);
	constexpr EvalArray QUEEN_BLACK = reverseArray(QUEEN_WHITE);
	constexpr EvalArray KING_BLACK = reverseArray(KING_WHITE);
}
