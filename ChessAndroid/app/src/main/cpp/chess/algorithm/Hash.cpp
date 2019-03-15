#include "Hash.h"

#include <random>

#include "../data/Board.h"

Hash::HashArray Hash::array{};
std::array<U64, 4> Hash::castlingRights;
U64 Hash::whiteToMove;

void Hash::init()
{
	if (initialized) return;
	initialized = true;

	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<U64> dist(0, UINT64_MAX);

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			for (byte p = 0; p < 8; p++)
                array[x][y][p] = dist(mt);

    for (byte i = 0; i < 4; i++)
        castlingRights[i] = dist(mt);

	whiteToMove = dist(mt);
}

U64 Hash::getHash(const Pos &pos, const Piece &piece)
{
	return array[pos.x][pos.y][indexOf(piece)];
}

U64 Hash::compute(const Board &board)
{
	U64 hash = 0;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
			if (const auto &piece = board.data[x][y]; piece)
				hash ^= array[x][y][indexOf(piece)];

	if (board.whiteToMove)
		hash ^= whiteToMove;

	return hash;
}

byte Hash::indexOf(const Piece& piece)
{
	byte type = static_cast<byte>(piece.type) - 1u;
	if (piece.isWhite) type += 6u;
	return type;
}

void Hash::makeMove(U64 &key, const Pos &selectedPos, const Pos &destPos, const Piece &selectedPiece, const Piece &destPiece)
{
	// Remove Selected Piece
	key ^= Hash::getHash(selectedPos, selectedPiece);

	if (destPiece) // Remove Destination Piece if any
		key ^= Hash::getHash(destPos, destPiece);

	// Add Selected Piece to Destination
	key ^= Hash::getHash(destPos, selectedPiece);
}

void Hash::promotePawn(U64 &key, const Pos &startPos, const Pos &destPos, const bool isWhite)
{
	// Remove the Pawn
	key ^= array[startPos.x][startPos.y][indexOf(Piece(Type::PAWN, isWhite))];

	// Add Queen
	key ^= array[destPos.x][destPos.y][indexOf(Piece(Type::QUEEN, isWhite))];
}

void Hash::flipSide(U64 &key)
{
	key ^= Hash::whiteToMove;
}
