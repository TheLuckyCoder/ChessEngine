#include "Psqt.h"

#define S Score

constexpr S PAWN_SCORE(136, 208);
constexpr S KNIGHT_SCORE(782, 865);
constexpr S BISHOP_SCORE(830, 918);
constexpr S ROOK_SCORE(1289, 1378);
constexpr S QUEEN_SCORE(2529, 2687);

constexpr S PAWN_SQUARE[][4] =
{
	{ S(0,   0),  S(0,   0),   S(0,   0),  S(0,  0)  },
	{ S(-11, -3), S(7,   -1),  S(7,   7),  S(17, 2)  },
	{ S(-16, -2), S(-3,  2),   S(23,  6),  S(23, -1) },
	{ S(-14, 7),  S(-7,  -4),  S(20,  -8), S(24, 2)  },
	{ S(-5,  13), S(-2,  10),  S(-1,  -1), S(12, -8) },
	{ S(-11, 16), S(-12, 6),   S(-2,  1),  S(4,  16) },
	{ S(-2,  1),  S(20,  -12), S(-10, 6),  S(-2, 25) },
	{ S(0,   0),  S(0,   0),   S(0,   0),  S(0,  0)  }
};
constexpr S KNIGHT_SQUARE[][4] =
{
	{ S(-169, -105), S(-96, -74), S(-80, -46), S(-79, -18) },
	{ S(-79,  -70),  S(-39, -56), S(-24, -15), S(-9,  6)   },
	{ S(-64,  -38),  S(-20, -33), S(4,   -5),  S(19,  27)  },
	{ S(-28,  -36),  S(5,   0),   S(41,  13),  S(47,  34)  },
	{ S(-29,  -41),  S(13,  -20), S(42,  4),   S(52,  35)  },
	{ S(-11,  -51),  S(28,  -38), S(63,  -17), S(55,  19)  },
	{ S(-67,  -64),  S(-21, -45), S(6,   -37), S(37,  16)  },
	{ S(-200, -98),  S(-80, -89), S(-53, -53), S(-32, -16) }
};
constexpr S BISHOP_SQUARE[][4] =
{
	{ S(-44, -63), S(-4,  -30), S(-11, -35), S(-28, -8)  },
	{ S(-18, -38), S(7,   -13), S(14,  -14), S(3,   0)   },
	{ S(-8,  -18), S(24,  0),   S(-3,  -7),  S(15,  13)  },
	{ S(1,   -26), S(8,   -3),  S(26,  1),   S(37,  16)  },
	{ S(-7,  -24), S(30,  -6),  S(23,  -10), S(28,  17)  },
	{ S(-17, -26), S(4,   2),   S(-1,  1),   S(8,   16)  },
	{ S(-21, -34), S(-19, -18), S(10,  -7),  S(-6,  9)   },
	{ S(-48, -51), S(-3,  -40), S(-12, -39), S(-25, -20) }
};
constexpr S ROOK_SQUARE[][4] =
{
	{ S(-24, -2),  S(-13, -6), S(-7, -3),  S(2,-2)  },
	{ S(-18, -10), S(-10, -7), S(-5, 1),   S(9, 0)  },
	{ S(-21, 10),  S(-7,  -4), S(3,  2),   S(-1,-2) },
	{ S(-13, -5),  S(-5,  2),  S(-4, -8),  S(-6, 8) },
	{ S(-24, -8),  S(-12, 5),  S(-1, 4),   S(6, -9) },
	{ S(-24, 3),   S(-4,  -2), S(4,  -10), S(10, 7) },
	{ S(-8,  1),   S(6,   2),  S(10, 17),  S(12,-8) },
	{ S(-22, 12),  S(-24, -6), S(-6, 13),  S(4, 7)  }
};
constexpr S QUEEN_SQUARE[][4] =
{
	{ S(3,  -69), S(-5, -57), S(-5, -47), S(4,  -26) },
	{ S(-3, -55), S(5,  -31), S(8,  -22), S(12, -4)  },
	{ S(-3, -39), S(6,  -18), S(13, -9),  S(7,  3)   },
	{ S(4,  -23), S(5,  -3),  S(9,  13),  S(8,  24)  },
	{ S(0,  -29), S(14, -6),  S(12, 9),   S(5,  21)  },
	{ S(-4, -38), S(10, -18), S(6,  -12), S(8,  1)   },
	{ S(-5, -50), S(6,  -27), S(10, -24), S(8,  -8)  },
	{ S(-2, -75), S(-2, -52), S(1,  -43), S(-2, -36) }
};
constexpr S KING_SQUARE[][4] =
{
	{ S(272, 0),   S(325, 41),  S(273, 80),  S(190, 93)  },
	{ S(277, 57),  S(305, 98),  S(241, 138), S(183, 131) },
	{ S(198, 86),  S(253, 138), S(168, 165), S(120, 173) },
	{ S(169, 103), S(191, 152), S(136, 168), S(108, 169) },
	{ S(145, 98),  S(176, 166), S(112, 197), S(69,  194) },
	{ S(122, 87),  S(159, 164), S(85,  174), S(36,  189) },
	{ S(87,  40),  S(120, 99),  S(64,  128), S(25,  141) },
	{ S(64,  5),   S(87,  60),  S(49,  75),  S(0,   75)  }
};

#undef S

using byte = unsigned char;

const Psqt::ScoreArray Psqt::s_PawnSquares = [] {
	ScoreArray array{};

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			array[x][y] = PAWN_SCORE + PAWN_SQUARE[7u - x][std::min<byte>(y, 7u - y)];

	return array;
}();

const Psqt::ScoreArray Psqt::s_KnightSquares = [] {
	ScoreArray array{};

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			array[x][y] = KNIGHT_SCORE + KNIGHT_SQUARE[7u - x][std::min<byte>(y, 7u - y)];

	return array;
}();

const Psqt::ScoreArray Psqt::s_BishopSquares = [] {
	ScoreArray array{};

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			array[x][y] = BISHOP_SCORE + BISHOP_SQUARE[7u - x][std::min<byte>(y, 7u - y)];

	return array;
}();

const Psqt::ScoreArray Psqt::s_RookSquares = [] {
	ScoreArray array{};

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			array[x][y] = ROOK_SCORE + ROOK_SQUARE[7u - x][std::min<byte>(y, 7u - y)];

	return array;
}();

const Psqt::ScoreArray Psqt::s_QueenSquares = [] {
	ScoreArray array{};

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			array[x][y] = QUEEN_SCORE + QUEEN_SQUARE[7u - x][std::min<byte>(y, 7u - y)];

	return array;
}();

const Psqt::ScoreArray Psqt::s_KingSquares = [] {
	ScoreArray array{};

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			array[x][y] = KING_SQUARE[7u - x][std::min<byte>(y, 7u - y)];

	return array;
}();
