#pragma once

#include <array>

#include "Score.h"

class Psqt
{
	using ScoreArray = std::array<std::array<Score, 64>, 7>;

public:
	Psqt() = delete;
	Psqt(const Psqt&) = delete;
	Psqt(Psqt&&) = delete;

	const static ScoreArray s_Bonus;
};
