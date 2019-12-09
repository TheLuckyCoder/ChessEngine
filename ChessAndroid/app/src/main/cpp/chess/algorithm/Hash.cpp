#include "Hash.h"

#include <random>

#include "../data/Board.h"

Hash::HashArray Hash::s_Pieces{};
std::array<U64, 4> Hash::s_CastlingRights;
U64 Hash::s_WhiteToMove;

void Hash::init()
{
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<U64> dist(0, UINT64_MAX);

	for (auto &i : s_Pieces)
		for (auto &color : i)
			for (auto &piece : color)
				piece = dist(mt);

	for (auto &rights : s_CastlingRights)
		rights = dist(mt);

	s_WhiteToMove = dist(mt);
}

U64 Hash::movePiece(const byte sq, const Piece &piece)
{
	return s_Pieces[sq][piece.isWhite][piece.type];
}

U64 Hash::compute(const Board &board)
{
	U64 hash{};

	for (byte i = 0; i < 64; ++i)
		if (const Piece &piece = board.getPiece(i); piece)
			hash ^= s_Pieces[i][piece.isWhite][piece.type];

	if (board.colorToMove)
		hash ^= s_WhiteToMove;

	addCastlingRights(hash, static_cast<CastlingRights>(board.castlingRights));

	return hash;
}

void Hash::makeMove(U64 &key, const byte selectedSq, const byte destSq, const Piece &selectedPiece, const Piece &destPiece)
{
	// Remove Selected Piece
	key ^= Hash::movePiece(selectedSq, selectedPiece);

	if (destPiece) // Remove Destination Piece if any
		key ^= Hash::movePiece(destSq, destPiece);

	// Add Selected Piece to Destination
	key ^= Hash::movePiece(destSq, selectedPiece);
}

void Hash::promotePawn(U64 &key, const byte sq, const Color color, const PieceType promotedType)
{
	// Remove the Pawn
	key ^= s_Pieces[sq][color][PAWN];

	// Add the Promoted Piece
	key ^= s_Pieces[sq][color][promotedType];
}

void Hash::xorPiece(U64 &key, const byte sq, const Piece &piece)
{
	key ^= s_Pieces[sq][piece.isWhite][piece.type];
}

void Hash::flipSide(U64 &key)
{
	key ^= Hash::s_WhiteToMove;
}

void Hash::addCastlingRights(U64 &key, const CastlingRights rights)
{
	if (rights & CASTLE_WHITE_KING)
		key ^= s_CastlingRights[0];
	if (rights & CASTLE_WHITE_QUEEN)
		key ^= s_CastlingRights[1];

	if (rights & CASTLE_BLACK_KING)
		key ^= s_CastlingRights[2];
	if (rights & CASTLE_BLACK_QUEEN)
		key ^= s_CastlingRights[3];
}

void Hash::removeCastlingRights(U64 &key, const CastlingRights rights)
{
	// Remove them in the opposite order they were added in
	if (rights & CASTLE_BLACK_QUEEN)
		key ^= s_CastlingRights[3];
	if (rights & CASTLE_BLACK_KING)
		key ^= s_CastlingRights[2];

	if (rights & CASTLE_WHITE_QUEEN)
		key ^= s_CastlingRights[1];
	if (rights & CASTLE_WHITE_KING)
		key ^= s_CastlingRights[0];
}
