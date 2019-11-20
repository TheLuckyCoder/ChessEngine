#pragma once

#include <array>

#include "PosMap.h"
#include "StackVector.h"

using U64 = std::uint64_t;

class AttacksMap
{
public:
	std::array<std::array<U64, 6>, 2> board{}; // Color[2] and Piece Types[6]
	PosMap map;
};

template<std::size_t N>
using PosVector = StackVector<Pos, N>;
