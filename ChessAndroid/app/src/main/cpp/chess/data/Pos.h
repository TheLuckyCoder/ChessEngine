#pragma once

#include <utility>

class Pos
{
public:
	short x;
	short y;

	Pos()
		: x(-1), y(-1) {}

	Pos(short x, short y)
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

// Needed for std::unordered_map
namespace std
{

	template <>
	struct hash<Pos>
	{
		std::size_t operator()(const Pos &pos) const
		{
			return (std::hash<short>()(pos.x) << 1) ^ (std::hash<short>()(pos.y));
		}
	};

}
