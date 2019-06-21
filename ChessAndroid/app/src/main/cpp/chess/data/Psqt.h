#pragma once

#include <array>

#include "Score.h"

class Psqt
{
	using ScoreArray = std::array<std::array<Score, 8>, 8>;

public:
	Psqt() = delete;
	Psqt(const Psqt&) = delete;
	Psqt(Psqt&&) = delete;

	const static ScoreArray s_PawnSquares;
	const static ScoreArray s_KnightSquares;
	const static ScoreArray s_BishopSquares;
	const static ScoreArray s_RookSquares;
	const static ScoreArray s_QueenSquares;
	const static ScoreArray s_KingSquares;
};
