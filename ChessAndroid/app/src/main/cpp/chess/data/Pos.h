#pragma once

#include <utility>

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

	friend Pos operator+(Pos left, const Pos &right) noexcept;
	friend Pos operator-(Pos left, const Pos &right) noexcept;
	friend Pos operator*(Pos left, const Pos &right) noexcept;
	friend Pos operator/(Pos left, const Pos &right) noexcept;

	constexpr bool operator==(const Pos &other) const noexcept
	{
		return x == other.x && y == other.y;
	}
	constexpr bool operator!=(const Pos &other) const noexcept
	{
		return !(*this == other);
	}

	constexpr Pos& operator+=(const Pos &other) noexcept;
	constexpr Pos& operator-=(const Pos &other) noexcept;
	constexpr Pos& operator*=(const Pos &other) noexcept;
	constexpr Pos& operator/=(const Pos &other) noexcept;

	constexpr bool isValid() const noexcept
	{
		return x < 8 && y < 8;
	}

};

using PosPair = std::pair<Pos, Pos>;

namespace std
{

	// Needed for std::unordered_map and std::unordered_set
	template <>
	struct hash<Pos>
	{
		constexpr std::size_t operator()(const Pos &pos) const noexcept
		{
			return pos.x * 1000 + pos.y;
		}
	};

}
