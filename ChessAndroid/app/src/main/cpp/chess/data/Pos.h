#pragma once

#include "Bits.h"

class Pos
{
public:
	constexpr Pos() noexcept
		: x(8), y(8) {}

	constexpr explicit Pos(const byte square) noexcept
		: x(col(square)), y(row(square)) {}

	constexpr Pos(const byte x, const byte y) noexcept
		: x(x), y(y) {}

	constexpr bool operator==(const Pos &other) const noexcept
	{
		return x == other.x && y == other.y;
	}

	constexpr bool operator!=(const Pos &other) const noexcept
	{
		return !(*this == other);
	}

	constexpr Pos &operator+=(const Pos &other) noexcept
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	constexpr Pos &operator-=(const Pos &other) noexcept
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	constexpr Pos operator+(Pos other) const noexcept
	{
		other.x += x;
		other.y += y;

		return other;
	}

	constexpr Pos operator-(Pos other) const noexcept
	{
		other.x -= x;
		other.y -= y;

		return other;
	}

	constexpr bool isValid() const noexcept
	{
		return x < 8 && y < 8;
	}

	constexpr byte toSquare() const noexcept
	{
		return ::toSquare(x, y);
	}

	byte x;
	byte y;
};
