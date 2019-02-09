#pragma once

//#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING

#include "StackVector.h"
#include "sparsepp/spp.h"

class Pos;

template<std::size_t N>
using PosVector = StackVector<Pos, N>;
using PosMap = spp::sparse_hash_map<Pos, short, std::hash<Pos>>;
