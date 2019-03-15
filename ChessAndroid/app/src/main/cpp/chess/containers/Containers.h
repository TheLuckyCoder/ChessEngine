#pragma once

#include "PosMap.h"
#include "StackVector.h"

class Attacks
{
public:
	Bitboard board[2][6]{}; // Color[2] and Piece Types[6]
	PosMap map;
};

template<std::size_t N>
using PosVector = StackVector<Pos, N>;
