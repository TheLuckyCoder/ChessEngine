#pragma once

#include <utility>

using byte = unsigned char;

class Pos
{
public:
	byte x;
	byte y;

	Pos()
		: x(8), y(8) {}

	Pos(const byte x, const byte y)
		: x(x), y(y) {}

	bool isValid() const;

	friend Pos operator+(Pos left, const Pos &right);
	friend Pos operator-(Pos left, const Pos &right);
	friend Pos operator*(Pos left, const Pos &right);
	friend Pos operator/(Pos left, const Pos &right);

	bool operator==(const Pos &other) const;
	bool operator!=(const Pos &other) const;

	Pos& operator+=(const Pos &other);
	Pos& operator-=(const Pos &other);
	Pos& operator*=(const Pos &other);
	Pos& operator/=(const Pos &other);
};

using PosPair = std::pair<Pos, Pos>;

namespace std
{

	// Needed for std::unordered_map and std::unordered_set
	template <>
	struct hash<Pos>
	{
		std::size_t operator()(const Pos &pos) const
		{
			return (std::hash<unsigned char>()(pos.x) << 2) ^ (std::hash<unsigned char>()(pos.y));
		}
	};

}
