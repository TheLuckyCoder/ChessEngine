#pragma once

#include <array>

#include "Defs.h"
#include "Score.h"

namespace Psqt
{
	using ScoreArray = std::array<std::array<Score, 64>, 7>;

	constexpr ScoreArray BONUS = []
    {
        constexpr Score PAWN_SCORE{ 128, 213 };
        constexpr Score KNIGHT_SCORE{ 781, 854 };
        constexpr Score BISHOP_SCORE{ 825, 915 };
        constexpr Score ROOK_SCORE{ 1276, 1380 };
        constexpr Score QUEEN_SCORE{ 2538, 2682 };

        constexpr Score PAWN_SQUARE[][4]
        {
            { },
            { { -11, -3 }, {   7,  -1 }, {  7,   7 }, { 17,  2 } },
            { { -16, -2 }, {  -3,   2 }, {  23,  6 }, { 23, -1 } },
            { { -14,  7 }, {  -7,  -4 }, {  20, -8 }, { 24,  2 } },
            { {  -5, 13 }, {  -2,  10 }, {  -1, -1 }, { 12, -8 } },
            { { -11, 16 }, { -12,   6 }, {  -2,  1 }, {  4, 16 } },
            { {  -2,  1 }, {  20, -12 }, { -10,  6 }, { -2, 25 } },
            { }
        };

        constexpr Score KNIGHT_SQUARE[][4]
        {
            { { -175,  -96 }, { -92, -65 }, { -74, -49 }, { -73, -21 } },
            { {  -77,  -67 }, { -41, -54 }, { -27, -18 }, { -15,   8 } },
            { {  -61,  -40 }, { -17, -27 }, {   6,  -8 }, {  12,  29 } },
            { {  -35,  -35 }, {   8,  -2 }, {  40,  13 }, {  49,  28 } },
            { {  -34,  -45 }, {  13, -16 }, {  44,   9 }, {  51,  39 } },
            { {   -9,  -51 }, {  22, -44 }, {  58, -16 }, {  53,  17 } },
            { {  -67,  -69 }, { -27, -50 }, {   4, -51 }, {  37,  12 } },
            { { -201, -100 }, { -83, -88 }, { -56, -56 }, { -26, -17 } }
        };

        constexpr Score BISHOP_SQUARE[][4]
        {
            { { -53, -57 }, {  -5, -30 }, {  -8, -37 }, { -23, -12 } },
            { { -15, -37 }, {   8, -13 }, {  19, -17 }, {   4,   1 } },
            { {  -7, -16 }, {  21,  -1 }, {  -5,  -2 }, {  17,  10 } },
            { {  -5, -20 }, {  11,  -6 }, {  25,   0 }, {  39,  17 } },
            { { -12, -17 }, {  29,  -1 }, {  22, -14 }, {  31,  15 } },
            { { -16, -30 }, {   6,   6 }, {   1,   4 }, {  11,   6 } },
            { { -17, -31 }, { -14, -20 }, {   5,  -1 }, {   0,   1 } },
            { { -48, -46 }, {   1, -42 }, { -14, -37 }, { -23, -24 } }
        };

        constexpr Score ROOK_SQUARE[][4]
        {
            { { -31,  -9 }, { -20, -13 }, { -14, -10 }, { -5,  -9 } },
            { { -21, -12 }, { -13,  -9 }, {  -8,  -1 }, {  6,  -2 } },
            { { -25,   6 }, { -11,  -8 }, {  -1,  -2 }, {  3,  -6 } },
            { { -13,  -6 }, {  -5,   1 }, {  -4,  -9 }, { -6,   7 } },
            { { -27,  -5 }, { -15,   8 }, {  -4,   7 }, {  3,  -6 } },
            { { -22,   6 }, {  -2,   1 }, {   6,  -7 }, { 12,  10 } },
            { {  -2,   4 }, {  12,   5 }, {  16,  20 }, { 18,  -5 } },
            { { -17,  18 }, { -19,   0 }, {  -1,  19 }, {  9,  13 } }
        };

        constexpr Score QUEEN_SQUARE[][4]
        {
            { {  3, -69 }, { -5, -57 }, { -5, -47 }, {  4, -26 } },
            { { -3, -55 }, {  5, -31 }, {  8, -22 }, { 12,  -4 } },
            { { -3, -39 }, {  6, -18 }, { 13,  -9 }, {  7,   3 } },
            { {  4, -23 }, {  5,  -3 }, {  9,  13 }, {  8,  24 } },
            { {  0, -29 }, { 14,  -6 }, { 12,   9 }, {  5,  21 } },
            { { -4, -38 }, { 10, -18 }, {  6, -12 }, {  8,   1 } },
            { { -5, -50 }, {  6, -27 }, { 10, -24 }, {  8,  -8 } },
            { { -2, -75 }, { -2, -52 }, {  1, -43 }, { -2, -36 } }
        };

        constexpr Score KING_SQUARE[][4]
        {
            { { 271,   1 }, { 327,  45 }, { 270,  85 }, { 192,  76 } },
            { { 278,  53 }, { 303, 100 }, { 230, 133 }, { 174, 135 } },
            { { 195,  88 }, { 258, 130 }, { 169, 169 }, { 120, 175 } },
            { { 164, 103 }, { 190, 156 }, { 138, 172 }, {  98, 172 } },
            { { 154,  96 }, { 179, 166 }, { 105, 199 }, {  70, 199 } },
            { { 123,  92 }, { 145, 172 }, {  81, 184 }, {  31, 191 } },
            { {  88,  47 }, { 120, 121 }, {  65, 116 }, {  33, 131 } },
            { {  59,  11 }, {  89,  59 }, {  45,  73 }, {  -1,  78 } }
        };

        ScoreArray bonuses{};

        for (byte i{}; i < SQUARE_NB; ++i)
        {
            const byte x = col(i);
            const byte y = row(i);
            const byte queen_side_y = std::min<byte>(y, 7u - y);

            bonuses[PAWN][i] = PAWN_SCORE + PAWN_SQUARE[x][queen_side_y];
            bonuses[KNIGHT][i] = KNIGHT_SCORE + KNIGHT_SQUARE[x][queen_side_y];
            bonuses[BISHOP][i] = BISHOP_SCORE + BISHOP_SQUARE[x][queen_side_y];
            bonuses[ROOK][i] = ROOK_SCORE + ROOK_SQUARE[x][queen_side_y];
            bonuses[QUEEN][i] = QUEEN_SCORE + QUEEN_SQUARE[x][queen_side_y];
            bonuses[KING][i] = KING_SQUARE[x][queen_side_y];
        }

        return bonuses;
    }();
}
