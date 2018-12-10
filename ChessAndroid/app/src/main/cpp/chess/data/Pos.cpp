#include "Pos.h"

bool Pos::isValid() const
{
	return x < 8 && x >= 0 && y < 8 && y >= 0;
}

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

Pos &Pos::operator+=(const Pos &other)
{
	x += other.x;
	y += other.y;

	return *this;
}

Pos &Pos::operator-=(const Pos &other)
{
	x -= other.x;
	y -= other.y;

	return *this;
}

Pos &Pos::operator*=(const Pos &other)
{
	x *= other.x;
	y *= other.y;

	return *this;
}

Pos &Pos::operator/=(const Pos &other)
{
	x /= other.x;
	y /= other.y;

	return *this;
}
