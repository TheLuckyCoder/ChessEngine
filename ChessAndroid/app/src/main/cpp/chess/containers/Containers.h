#pragma once

//#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING

#include "StackVector.h"
#include "sparsepp/spp.h"

class Pos;

template<std::size_t N>
using PosVector = StackVector<Pos, N>;
using PosMap = spp::sparse_hash_map<Pos, short, std::hash<Pos>>;
using PosSet = spp::sparse_hash_set<Pos, std::hash<Pos>>;

template<class T>
bool exists(const T &t, const Pos &pos)
{
	const auto iterator = std::find(t.begin(), t.end(), pos);
	return iterator != t.end();
}
