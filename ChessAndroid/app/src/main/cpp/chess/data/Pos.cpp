#include "Pos.h"

Pos operator+(Pos left, const Pos &right) noexcept
{
	return left += right;
}

Pos operator-(Pos left, const Pos &right) noexcept
{
	return left -= right;
}

Pos operator*(Pos left, const Pos &right) noexcept
{
	return left *= right;
}

Pos operator/(Pos left, const Pos &right) noexcept
{
	return left /= right;
}

constexpr Pos &Pos::operator+=(const Pos &other) noexcept
{
	x += other.x;
	y += other.y;

	return *this;
}

constexpr Pos &Pos::operator-=(const Pos &other) noexcept
{
	x -= other.x;
	y -= other.y;

	return *this;
}

constexpr Pos &Pos::operator*=(const Pos &other) noexcept
{
	x *= other.x;
	y *= other.y;

	return *this;
}

constexpr Pos &Pos::operator/=(const Pos &other) noexcept
{
	x /= other.x;
	y /= other.y;

	return *this;
}
