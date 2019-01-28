#pragma once

#include <utility>

using byte = unsigned char;

class Pos
{
public:
	byte x;
	byte y;

	constexpr Pos()
		: x(8), y(8) {}

	constexpr Pos(const byte x, const byte y)
		: x(x), y(y) {}

	constexpr Pos(const Pos pos, const byte x, const byte y)
		: x(pos.x + x), y(pos.y + y) {}

	constexpr bool isValid() const
	{
		return x < 8 && y < 8;
	}

	friend Pos operator+(Pos left, const Pos &right);
	friend Pos operator-(Pos left, const Pos &right);
	friend Pos operator*(Pos left, const Pos &right);
	friend Pos operator/(Pos left, const Pos &right);

	bool operator==(const Pos &other) const;
	bool operator!=(const Pos &other) const;

	constexpr Pos& operator+=(const Pos &other);
	constexpr Pos& operator-=(const Pos &other);
	constexpr Pos& operator*=(const Pos &other);
	constexpr Pos& operator/=(const Pos &other);
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
			return (std::hash<unsigned char>()(pos.x) << 2) ^ (std::hash<unsigned char>()(pos.y));
		}
	};

}
