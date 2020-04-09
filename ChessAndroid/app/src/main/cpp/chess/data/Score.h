#pragma once

class Score final
{
public:
	short mg{}, eg{};

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
};

constexpr Score operator*(Score score, const short rhs) noexcept
{
	score.mg *= rhs;
	score.eg *= rhs;
	return score;
}
