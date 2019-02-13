#pragma once

class Pos;

class PosMap
{
    unsigned char _array[64]{};
public:

    constexpr unsigned char &operator[](const Pos &pos) noexcept
    {
        return _array[pos.x * 8u + pos.y];
    }

    constexpr const unsigned char &operator[](const Pos &pos) const noexcept
    {
        return _array[pos.x * 8u + pos.y];
    }
};
