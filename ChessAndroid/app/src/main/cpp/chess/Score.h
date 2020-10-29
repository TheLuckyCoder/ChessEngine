#pragma once

class Score final
{
public:
	Score() = default;

	constexpr Score(const short mg, const short eg) noexcept
		: mg(mg), eg(eg) {}

	constexpr Score &operator=(const short rhs) noexcept
	{
		mg = rhs;
		eg = rhs;
		return *this;
	}

	constexpr void operator+=(const Score &rhs) noexcept
	{
		mg += rhs.mg;
		eg += rhs.eg;
	}

	constexpr void operator-=(const Score &rhs) noexcept
	{
		mg -= rhs.mg;
		eg -= rhs.eg;
	}

	constexpr void operator+=(const short rhs) noexcept
	{
		mg += rhs;
		eg += rhs;
	}

	constexpr void operator-=(const short rhs) noexcept
	{
		mg -= rhs;
		eg -= rhs;
	}

	constexpr Score operator+(const Score &rhs) const noexcept
	{
		Score lhs = *this;
		lhs.mg += rhs.mg;
		lhs.eg += rhs.eg;
		return lhs;
	}

	constexpr Score operator-(const Score &rhs) const noexcept
	{
		Score lhs = *this;
		lhs.mg -= rhs.mg;
		lhs.eg -= rhs.eg;
		return lhs;
	}

	short mg{}, eg{};
};

constexpr Score operator*(Score score, const short rhs) noexcept
{
	score.mg *= rhs;
	score.eg *= rhs;
	return score;
}
