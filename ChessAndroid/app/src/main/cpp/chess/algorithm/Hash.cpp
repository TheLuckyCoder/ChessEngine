#include "Hash.h"

#include <random>

#include "../data/Board.h"

Hash::HashArray Hash::array{};
Hash::Key Hash::whiteToMove;

byte Hash::indexOf(const Piece& piece)
{
	byte type = static_cast<byte>(piece.type) - 1;
	if (piece.isWhite) type += 6;
	return type;
}

void Hash::initHashKeys()
{
	if (initialized) return;
	initialized = true;

	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<Key> dist(0, UINT64_MAX);

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			for (byte p = 0; p < 8; p++)
				for (byte m = 0; m < 2; m++)
					array[x][y][p][m] = dist(mt);

	whiteToMove = dist(mt);
}

Hash::Key Hash::getHash(const Pos &pos, const Piece &piece)
{
	return array[pos.x][pos.y][indexOf(piece)][piece.moved];
}

Hash::Key Hash::compute(const Board &board)
{
	Key hash = 0;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece)
				hash ^= array[x][y][indexOf(piece)][piece.moved];

	if (whiteToMove)
		hash ^= whiteToMove;

	return hash;
}
