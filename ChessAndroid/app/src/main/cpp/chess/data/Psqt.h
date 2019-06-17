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

	const static ScoreArray PAWN_SQUARE;
	const static ScoreArray KNIGHT_SQUARE;
	const static ScoreArray BISHOP_SQUARE;
	const static ScoreArray ROOK_SQUARE;
	const static ScoreArray QUEEN_SQUARE;
	const static ScoreArray KING_SQUARE;
};
