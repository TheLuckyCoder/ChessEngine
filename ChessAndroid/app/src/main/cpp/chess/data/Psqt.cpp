#include "Psqt.h"

#include "Bitboard.h"

#define S Score

constexpr static S PAWN_SCORE(128, 213);
constexpr static S KNIGHT_SCORE(781, 854);
constexpr static S BISHOP_SCORE(825, 915);
constexpr static S ROOK_SCORE(1276, 1380);
constexpr static S QUEEN_SCORE(2538, 2682);

constexpr static S PAWN_SQUARE[][4] = {
	{ S(0,   0),  S(0,   0),   S(0,   0),  S(0,  0)  },
	{ S(-11, -3), S(7,   -1),  S(7,   7),  S(17, 2)  },
	{ S(-16, -2), S(-3,  2),   S(23,  6),  S(23, -1) },
	{ S(-14, 7),  S(-7,  -4),  S(20,  -8), S(24, 2)  },
	{ S(-5,  13), S(-2,  10),  S(-1,  -1), S(12, -8) },
	{ S(-11, 16), S(-12, 6),   S(-2,  1),  S(4,  16) },
	{ S(-2,  1),  S(20,  -12), S(-10, 6),  S(-2, 25) },
	{ S(0,   0),  S(0,   0),   S(0,   0),  S(0,  0)  }
};

constexpr static S KNIGHT_SQUARE[][4] = {
	{ S(-175,  -96), S(-92, -65), S(-74, -49), S(-73, -21) },
	{ S( -77,  -67), S(-41, -54), S(-27, -18), S(-15,   8) },
	{ S( -61,  -40), S(-17, -27), S(  6,  -8), S( 12,  29) },
	{ S( -35,  -35), S(  8,  -2), S( 40,  13), S( 49,  28) },
	{ S( -34,  -45), S( 13, -16), S( 44,   9), S( 51,  39) },
	{ S(  -9,  -51), S( 22, -44), S( 58, -16), S( 53,  17) },
	{ S( -67,  -69), S(-27, -50), S(  4, -51), S( 37,  12) },
	{ S(-201, -100), S(-83, -88), S(-56, -56), S(-26, -17) }
};

constexpr static S BISHOP_SQUARE[][4] = {
	{ S(-53, -57), S( -5, -30), S( -8, -37), S(-23, -12) },
	{ S(-15, -37), S(  8, -13), S( 19, -17), S(  4,   1) },
	{ S( -7, -16), S( 21,  -1), S( -5,  -2), S( 17,  10) },
	{ S( -5, -20), S( 11,  -6), S( 25,   0), S( 39,  17) },
	{ S(-12, -17), S( 29,  -1), S( 22, -14), S( 31,  15) },
	{ S(-16, -30), S(  6,   6), S(  1,   4), S( 11,   6) },
	{ S(-17, -31), S(-14, -20), S(  5,  -1), S(  0,   1) },
	{ S(-48, -46), S(  1, -42), S(-14, -37), S(-23, -24) }
};

constexpr static S ROOK_SQUARE[][4] = {
	{ S(-31,  -9), S(-20, -13), S(-14, -10), S(-5,  -9) },
	{ S(-21, -12), S(-13,  -9), S( -8,  -1), S( 6,  -2) },
	{ S(-25,   6), S(-11,  -8), S( -1,  -2), S( 3,  -6) },
	{ S(-13,  -6), S( -5,   1), S( -4,  -9), S(-6,   7) },
	{ S(-27,  -5), S(-15,   8), S( -4,   7), S( 3,  -6) },
	{ S(-22,   6), S( -2,   1), S(  6,  -7), S(12,  10) },
	{ S( -2,   4), S( 12,   5), S( 16,  20), S(18,  -5) },
	{ S(-17,  18), S(-19,   0), S( -1,  19), S( 9,  13) }
};

constexpr static S QUEEN_SQUARE[][4] = {
	{ S( 3, -69), S(-5, -57), S(-5, -47), S( 4, -26) },
	{ S(-3, -55), S( 5, -31), S( 8, -22), S(12,  -4) },
	{ S(-3, -39), S( 6, -18), S(13,  -9), S( 7,   3) },
	{ S( 4, -23), S( 5,  -3), S( 9,  13), S( 8,  24) },
	{ S( 0, -29), S(14,  -6), S(12,   9), S( 5,  21) },
	{ S(-4, -38), S(10, -18), S( 6, -12), S( 8,   1) },
	{ S(-5, -50), S( 6, -27), S(10, -24), S( 8,  -8) },
	{ S(-2, -75), S(-2, -52), S( 1, -43), S(-2, -36) }
};

constexpr static S KING_SQUARE[][4] = {
	{ S(271,   1), S(327,  45), S(270,  85), S(192,  76) },
	{ S(278,  53), S(303, 100), S(230, 133), S(174, 135) },
	{ S(195,  88), S(258, 130), S(169, 169), S(120, 175) },
	{ S(164, 103), S(190, 156), S(138, 172), S( 98, 172) },
	{ S(154,  96), S(179, 166), S(105, 199), S( 70, 199) },
	{ S(123,  92), S(145, 172), S( 81, 184), S( 31, 191) },
	{ S( 88,  47), S(120, 121), S( 65, 116), S( 33, 131) },
	{ S( 59,  11), S( 89,  59), S( 45,  73), S( -1,  78) }
};

#undef S

using byte = unsigned char;

const Psqt::ScoreArray Psqt::s_PawnSquares = [] {
	ScoreArray array{};

	for (byte i = 0; i < 64; i++)
		array[i] = PAWN_SCORE + PAWN_SQUARE[col(i)][std::min<byte>(row(i), 7u - row(i))];

	return array;
}();

const Psqt::ScoreArray Psqt::s_KnightSquares = [] {
	ScoreArray array{};

	for (byte i = 0; i < 64; i++)
		array[i] = KNIGHT_SCORE + KNIGHT_SQUARE[col(i)][std::min<byte>(row(i), 7u - row(i))];

	return array;
}();

const Psqt::ScoreArray Psqt::s_BishopSquares = [] {
	ScoreArray array{};

	for (byte i = 0; i < 64; i++)
		array[i] = BISHOP_SCORE + BISHOP_SQUARE[col(i)][std::min<byte>(row(i), 7u - row(i))];

	return array;
}();

const Psqt::ScoreArray Psqt::s_RookSquares = [] {
	ScoreArray array{};

	for (byte i = 0; i < 64; i++)
		array[i] = ROOK_SCORE + ROOK_SQUARE[col(i)][std::min<byte>(row(i), 7u - row(i))];

	return array;
}();

const Psqt::ScoreArray Psqt::s_QueenSquares = [] {
	ScoreArray array{};

	for (byte i = 0; i < 64; i++)
		array[i] = QUEEN_SCORE + QUEEN_SQUARE[col(i)][std::min<byte>(row(i), 7u - row(i))];

	return array;
}();

const Psqt::ScoreArray Psqt::s_KingSquares = [] {
	ScoreArray array{};

	for (byte i = 0; i < 64; i++)
		array[i] = KING_SQUARE[col(i)][std::min<byte>(row(i), 7u - row(i))];

	return array;
}();
