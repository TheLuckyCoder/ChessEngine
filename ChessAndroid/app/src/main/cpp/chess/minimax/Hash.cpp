#include "Hash.h"

#include <random>

#include "../data/Board.h"

const Hash::HashArray Hash::array = getRandomHashArray();

Hash::HashArray Hash::getRandomHashArray()
{
	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<std::uint64_t> dist(0, -1);

	HashArray arr;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			for (byte p = 0; p < 8; p++)
				for (byte m = 0; m < 2; m++)
					arr[x][y][p][m] = dist(mt);

	return arr;
}

byte Hash::indexOf(const Piece& piece)
{
	byte type = static_cast<byte>(piece.type) - 1;
	if (piece.isWhite) type += 6;
	return type;
}

std::uint64_t Hash::getHash(const Pos & pos, const Piece & piece)
{
	return array[pos.x][pos.y][indexOf(piece)][piece.moved];
}

std::uint64_t Hash::compute(const Board& board)
{
	std::uint64_t hash = 0;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece)
				hash ^= array[x][y][indexOf(piece)][piece.moved];

	return hash;
}
