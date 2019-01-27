#pragma once

#include <unordered_set>

#include "StackVector.h"
#include "PoolAllocator.h"

template<typename T>
using PoolUnorderedSet = std::unordered_set<T, std::hash<T>, std::equal_to<T>, PoolAllocator<T>>;
