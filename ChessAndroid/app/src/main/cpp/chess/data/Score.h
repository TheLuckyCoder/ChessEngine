#pragma once

class Score final
{
public:
	short mg = 0, eg = 0;

	Score() = default;

	constexpr Score(const short mg, const short eg)
		: mg(mg), eg(eg) {}

	constexpr void operator+=(const Score &other)
	{
		mg += other.mg;
		eg += other.eg;
	}

	constexpr void operator-=(const Score &other)
	{
		mg -= other.mg;
		eg -= other.eg;
	}

	constexpr void operator+=(const short score)
	{
		mg += score;
		eg += score;
	}

	constexpr void operator-=(const short score)
	{
		mg -= score;
		eg -= score;
	}
};
