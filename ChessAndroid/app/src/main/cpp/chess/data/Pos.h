#pragma once

#include <utility>

#include "Bitboard.h"

using byte = unsigned char;

class Pos
{
public:
	byte x;
	byte y;

	constexpr Pos() noexcept
		: x(8), y(8) {}

	constexpr Pos(const byte x, const byte y) noexcept
		: x(x), y(y) {}

	constexpr Pos(const Pos pos, const byte x, const byte y) noexcept
		: x(pos.x + x), y(pos.y + y) {}

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
	constexpr Pos &operator*=(const Pos &other) noexcept
	{
		x *= other.x;
		y *= other.y;

		return *this;
	}
	constexpr Pos &operator/=(const Pos &other) noexcept
	{
		x /= other.x;
		y /= other.y;

		return *this;
	}

	constexpr bool isValid() const noexcept
	{
		return x < 8 && y < 8;
	}

	constexpr byte toSquare() const noexcept
	{
		return x * 8u + y;
	}

	constexpr U64 toBitboard() const noexcept
	{
		return Bitboard::shiftedBoards[toSquare()];
	}
};

using PosPair = std::pair<Pos, Pos>;
