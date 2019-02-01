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

	constexpr bool isValid() const noexcept
	{
		return x < 8 && y < 8;
	}

	friend Pos operator+(Pos left, const Pos &right) noexcept;
	friend Pos operator-(Pos left, const Pos &right) noexcept;
	friend Pos operator*(Pos left, const Pos &right) noexcept;
	friend Pos operator/(Pos left, const Pos &right) noexcept;

	bool operator==(const Pos &other) const noexcept;
	bool operator!=(const Pos &other) const noexcept;

	constexpr Pos& operator+=(const Pos &other) noexcept;
	constexpr Pos& operator-=(const Pos &other) noexcept;
	constexpr Pos& operator*=(const Pos &other) noexcept;
	constexpr Pos& operator/=(const Pos &other) noexcept;
};

using PosPair = std::pair<Pos, Pos>;

namespace std
{

	// Needed for std::unordered_map and std::unordered_set
	template <>
	struct hash<Pos>
	{
		std::size_t operator()(const Pos &pos) const noexcept
		{
			return pos.x * 1000 + pos.y;
		}
	};

}
