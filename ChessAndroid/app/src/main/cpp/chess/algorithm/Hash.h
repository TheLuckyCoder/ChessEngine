#pragma once

#include <array>

#include "../data/Pos.h"
#include "../data/Piece.h"

class Hash final
{
public:
	using Key = std::uint64_t;

private:
	using HashArray = std::array<std::array<std::array<std::array<Key, 2>, 12>, 8>, 8>;
	static HashArray array;
	inline static bool initialized = false;

public:
	static Key whiteToMove;

	Hash() = delete;

private:
	static byte indexOf(const Piece &piece);

public:
	static void initHashKeys();
	static Key getHash(const Pos &pos, const Piece &piece);
	static Key compute(const Board &board);
};
