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

bool Pos::operator==(const Pos &other) const noexcept
{
	return x == other.x && y == other.y;
}

bool Pos::operator!=(const Pos &other) const noexcept
{
	return !(*this == other);
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
