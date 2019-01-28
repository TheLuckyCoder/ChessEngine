#include "Pos.h"

Pos operator+(Pos  left, const Pos &right)
{
	return left += right;
}

Pos operator-(Pos left, const Pos &right)
{
	return left -= right;
}

Pos operator*(Pos left, const Pos &right)
{
	return left *= right;
}

Pos operator/(Pos left, const Pos &right)
{
	return left /= right;
}

bool Pos::operator==(const Pos &other) const
{
	return x == other.x && y == other.y;
}

bool Pos::operator!=(const Pos &other) const
{
	return !(*this == other);
}

constexpr Pos &Pos::operator+=(const Pos &other)
{
	x += other.x;
	y += other.y;

	return *this;
}

constexpr Pos &Pos::operator-=(const Pos &other)
{
	x -= other.x;
	y -= other.y;

	return *this;
}

constexpr Pos &Pos::operator*=(const Pos &other)
{
	x *= other.x;
	y *= other.y;

	return *this;
}

constexpr Pos &Pos::operator/=(const Pos &other)
{
	x /= other.x;
	y /= other.y;

	return *this;
}
