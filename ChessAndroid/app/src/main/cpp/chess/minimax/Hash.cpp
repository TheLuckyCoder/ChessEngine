#include "Hash.h"

std::uint64_t Hash::compute(std::uint64_t previousHash, const Pos &selectedPos, const Pos &destPos, const Piece &selectedPiece, const Piece &destPiece)
{
	// TODO: Fix Hashing
	const int selectedIndex = indexOf(selectedPiece);

	previousHash ^= hashArray[selectedPos.x][selectedPos.y][selectedIndex];
	previousHash ^= hashArray[destPos.x][destPos.y][selectedIndex];

	if (destPiece)
		previousHash ^= hashArray[destPos.x][destPos.y][indexOf(destPiece)];

	return previousHash;
}
