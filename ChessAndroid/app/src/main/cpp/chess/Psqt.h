#pragma once

#include <array>

#include "Defs.h"
#include "Score.h"

constexpr auto PSQT = []
{
	constexpr Score PAWN_SCORE{ 128, 213 };
	constexpr Score KNIGHT_SCORE{ 781, 854 };
	constexpr Score BISHOP_SCORE{ 825, 915 };
	constexpr Score ROOK_SCORE{ 1276, 1380 };
	constexpr Score QUEEN_SCORE{ 2538, 2682 };

	constexpr Score PAWN_SQUARE[][4] = {
		{},
		{{ 3,  -10 }, { 3,   -6 },  { 10,  10 },  { 19,  0 }},
		{{ -9, -10 }, { -15, -10 }, { 11,  -10 }, { 15,  4 }},
		{{ -8, 6 },   { -23, -2 },  { 6,   -8 },  { 20,  -4 }},
		{{ 13, 9 },   { 0,   4 },   { -13, 3 },   { 1,   -12 }},
		{{ -5, 28 },  { -12, 20 },  { -7,  21 },  { 22,  28 }},
		{{ -7, 0 },   { 7,   -11 }, { -3,  12 },  { -13, 21 }},
		{}
	};

	constexpr Score KNIGHT_SQUARE[][4] = {
		{{ -175, -96 },  { -92, -65 }, { -74, -49 }, { -73, -21 }},
		{{ -77,  -67 },  { -41, -54 }, { -27, -18 }, { -15, 8 }},
		{{ -61,  -40 },  { -17, -27 }, { 6,   -8 },  { 12,  29 }},
		{{ -35,  -35 },  { 8,   -2 },  { 40,  13 },  { 49,  28 }},
		{{ -34,  -45 },  { 13,  -16 }, { 44,  9 },   { 51,  39 }},
		{{ -9,   -51 },  { 22,  -44 }, { 58,  -16 }, { 53,  17 }},
		{{ -67,  -69 },  { -27, -50 }, { 4,   -51 }, { 37,  12 }},
		{{ -201, -100 }, { -83, -88 }, { -56, -56 }, { -26, -17 }}
	};

	constexpr Score BISHOP_SQUARE[][4] = {
		{{ -37, -40 }, { -4,  -21 }, { -6,  -26 }, { -16, -8 }},
		{{ -11, -26 }, { 6,   -9 },  { 13,  -12 }, { 3,   1 }},
		{{ -5,  -11 }, { 15,  -1 },  { -4,  -1 },  { 12,  7 }},
		{{ -4,  -14 }, { 8,   -4 },  { 18,  0 },   { 27,  12 }},
		{{ -8,  -12 }, { 20,  -1 },  { 15,  -10 }, { 22,  11 }},
		{{ -11, -21 }, { 4,   4 },   { 1,   3 },   { 8,   4 }},
		{{ -12, -22 }, { -10, -14 }, { 4,   -1 },  { 0,   1 }},
		{{ -34, -32 }, { 1,   -29 }, { -10, -26 }, { -16, -17 }}
	};

	constexpr Score ROOK_SQUARE[][4] = {
		{{ -31, -9 },  { -20, -13 }, { -14, -10 }, { -5, -9 }},
		{{ -21, -12 }, { -13, -9 },  { -8,  -1 },  { 6,  -2 }},
		{{ -25, 6 },   { -11, -8 },  { -1,  -2 },  { 3,  -6 }},
		{{ -13, -6 },  { -5,  1 },   { -4,  -9 },  { -6, 7 }},
		{{ -27, -5 },  { -15, 8 },   { -4,  7 },   { 3,  -6 }},
		{{ -22, 6 },   { -2,  1 },   { 6,   -7 },  { 12, 10 }},
		{{ -2,  4 },   { 12,  5 },   { 16,  20 },  { 18, -5 }},
		{{ -17, 18 },  { -19, 0 },   { -1,  19 },  { 9,  13 }}
	};

	constexpr Score QUEEN_SQUARE[][4] = {
		{{ 3,  -69 }, { -5, -57 }, { -5, -47 }, { 4,  -26 }},
		{{ -3, -55 }, { 5,  -31 }, { 8,  -22 }, { 12, -4 }},
		{{ -3, -39 }, { 6,  -18 }, { 13, -9 },  { 7,  3 }},
		{{ 4,  -23 }, { 5,  -3 },  { 9,  13 },  { 8,  24 }},
		{{ 0,  -29 }, { 14, -6 },  { 12, 9 },   { 5,  21 }},
		{{ -4, -38 }, { 10, -18 }, { 6,  -12 }, { 8,  1 }},
		{{ -5, -50 }, { 6,  -27 }, { 10, -24 }, { 8,  -8 }},
		{{ -2, -75 }, { -2, -52 }, { 1,  -43 }, { -2, -36 }}
	};

	constexpr Score KING_SQUARE[][4] = {
		{{ 271, 1 },   { 327, 45 },  { 270, 85 },  { 192, 76 }},
		{{ 278, 53 },  { 303, 100 }, { 230, 133 }, { 174, 135 }},
		{{ 195, 88 },  { 258, 130 }, { 169, 169 }, { 120, 175 }},
		{{ 164, 103 }, { 190, 156 }, { 138, 172 }, { 98,  172 }},
		{{ 154, 96 },  { 179, 166 }, { 105, 199 }, { 70,  199 }},
		{{ 123, 92 },  { 145, 172 }, { 81,  184 }, { 31,  191 }},
		{{ 88,  47 },  { 120, 121 }, { 65,  116 }, { 33,  131 }},
		{{ 59,  11 },  { 89,  59 },  { 45,  73 },  { -1,  78 }}
	};

	std::array<std::array<Score, SQUARE_NB>, 7> bonuses{};

	for (u8 i{}; i < SQUARE_NB; ++i)
	{
		const u8 x = fileOf(i);
		const u8 y = rankOf(i);
		const u8 queenSideY = std::min<u8>(y, 7u - y);

		bonuses[PAWN][i] = PAWN_SCORE + PAWN_SQUARE[x][queenSideY];
		bonuses[KNIGHT][i] = KNIGHT_SCORE + KNIGHT_SQUARE[x][queenSideY];
		bonuses[BISHOP][i] = BISHOP_SCORE + BISHOP_SQUARE[x][queenSideY];
		bonuses[ROOK][i] = ROOK_SCORE + ROOK_SQUARE[x][queenSideY];
		bonuses[QUEEN][i] = QUEEN_SCORE + QUEEN_SQUARE[x][queenSideY];
		bonuses[KING][i] = KING_SQUARE[x][queenSideY];
	}

	return bonuses;
}();
