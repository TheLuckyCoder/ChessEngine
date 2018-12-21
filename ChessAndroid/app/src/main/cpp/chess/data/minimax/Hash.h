#pragma once

#include <array>

#include "Random.h"
#include "../Board.h"

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

	constexpr int indexOf(const Piece &piece)
	{
		const int type = static_cast<int>(piece.type) - 1;
		return piece.isWhite ? type : type + 6;
	}

	constexpr std::uint64_t compute(const Board &board)
	{
		std::uint64_t hash = 0;

		for (short i = 0; i < 8; i++)
			for (short j = 0; j < 8; j++)
				if (board.data[i][j])
					hash ^= hashArray[i][j][indexOf(board.data[i][j])];

		return hash;
	}

	std::uint64_t compute(std::uint64_t previousHash, const Pos &selectedPos, const Pos &destPos, const Piece &selectedPiece, const Piece &destPiece);
}
