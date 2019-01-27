#pragma once

#include <array>

#include "../data/pieces/Piece.h"

class Hash final
{
	using HashArray = std::array<std::array<std::array<std::array<std::uint64_t, 2>, 12>, 8>, 8>;
	const static HashArray array;

public:
	Hash() = delete;

private:
	static HashArray getRandomHashArray();
	static byte indexOf(const Piece &piece);

public:
	static std::uint64_t getHash(const Pos &pos, const Piece &piece);

	static std::uint64_t compute(const Board &board);
};
