#pragma once

class Score final
{
public:
	short mg = 0, eg = 0;

	Score() = default;

	constexpr Score(const short mg, const short eg) noexcept
		: mg(mg), eg(eg) {}

	constexpr void operator+=(const Score &other) noexcept
	{
		mg += other.mg;
		eg += other.eg;
	}

	constexpr void operator-=(const Score &other) noexcept
	{
		mg -= other.mg;
		eg -= other.eg;
	}

	constexpr void operator+=(const short score) noexcept
	{
		mg += score;
		eg += score;
	}

	constexpr void operator-=(const short score) noexcept
	{
		mg -= score;
		eg -= score;
	}
};

constexpr Score operator*(Score score, const int rhs) noexcept
{
	score.mg *= rhs;
	score.eg *= rhs;
	return score;
}
