#pragma once

using byte = unsigned char;

class Pos;

class PosMap
{
    unsigned char _array[64]{};

public:
    constexpr unsigned char &operator[](const byte square) noexcept
    {
        return _array[square];
    }

    constexpr const unsigned char &operator[](const byte square) const noexcept
    {
        return _array[square];
    }

    constexpr unsigned char &operator[](const Pos &pos) noexcept
    {
        return _array[pos.toSquare()];
    }

    constexpr const unsigned char &operator[](const Pos &pos) const noexcept
    {
        return _array[pos.toSquare()];
    }
};
