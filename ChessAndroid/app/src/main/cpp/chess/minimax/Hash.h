#pragma once

#include <array>

#include "Random.h"
#include "../data/Board.h"

namespace Hash
{
	static constexpr auto getRandomHashArray()
	{
		std::array<std::array<std::array<std::uint64_t, 12>, 8>, 8> arr{};
		PCG pcg;

		for (auto &i : arr)
			for (auto &j : i)
				for (auto &k : j)
					k = pcg();

		return arr;
	}

	constexpr auto hashArray = getRandomHashArray();

	constexpr byte indexOf(const Piece &piece)
	{
		auto type = static_cast<byte>(piece.type) - 1;
		if (piece.isWhite) type += 6;
		return type;
	}

	constexpr std::uint64_t compute(const Board &board)
	{
		std::uint64_t hash = 0;

		for (short x = 0; x < 8; x++)
			for (short y = 0; y < 8; y++)
				if (board.data[x][y])
					hash ^= hashArray[x][y][indexOf(board.data[x][y])];

		return hash;
	}

	std::uint64_t compute(std::uint64_t previousHash, const Pos &selectedPos, const Pos &destPos, const Piece &selectedPiece, const Piece &destPiece);
}
