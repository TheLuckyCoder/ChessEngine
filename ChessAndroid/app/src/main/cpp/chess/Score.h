#pragma once

class Score final
{
public:
	Score() = default;

	constexpr Score(const i16 mg, const i16 eg) noexcept
		: mg(mg), eg(eg) {}

	constexpr Score &operator=(const i16 rhs) noexcept
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

	constexpr void operator+=(const i16 rhs) noexcept
	{
		mg += rhs;
		eg += rhs;
	}

	constexpr void operator-=(const i16 rhs) noexcept
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

	constexpr Score operator*(const i16 rhs) const noexcept
	{
		Score lhs = *this;
		lhs.mg *= rhs;
		lhs.eg *= rhs;
		return lhs;
	}

	i16 mg{}, eg{};
};
