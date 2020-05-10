#include "Hash.h"

#include <random>

#include "../data/Board.h"

Hash::HashArray Hash::_pieces{};
U64 Hash::_side;
std::array<U64, 6> Hash::_castlingRights;
std::array<U64, 8> Hash::_enPassant;

void Hash::init()
{
	static bool initialized = false;
	if (initialized) return;
	initialized = true;

	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<U64> dist(0, UINT64_MAX);

	for (auto &i : _pieces)
		for (auto &color : i)
			for (auto &piece : color)
				piece = dist(mt);

	for (auto &rights : _castlingRights)
		rights = dist(mt);

	for (auto &square : _enPassant)
		square = dist(mt);

	_side = dist(mt);
}

U64 Hash::compute(const Board &board)
{
	U64 hash{};

	for (byte sq = 0; sq < SQUARE_NB; ++sq)
		if (const Piece &piece = board.getPiece(sq); piece)
			hash ^= _pieces[sq][piece.color()][piece.type()];

	xorCastlingRights(hash, static_cast<CastlingRights>(board.castlingRights));
	xorEnPassant(hash, board.enPassantSq);

	if (board.colorToMove)
		flipSide(hash);

	return hash;
}

void Hash::makeMove(U64 &key, const byte selectedSq, const byte destSq, const Piece &selectedPiece,
					const Piece &destPiece)
{
	// Remove Selected Piece
	xorPiece(key, selectedSq, selectedPiece);

	if (destPiece) // Remove Destination Piece if any
		xorPiece(key, destSq, destPiece);

	// Add Selected Piece to Destination
	xorPiece(key, destSq, selectedPiece);
}

void Hash::xorPiece(U64 &key, const byte sq, const Piece piece)
{
	key ^= _pieces[sq][piece.color()][piece.type()];
}

void Hash::flipSide(U64 &key)
{
	key ^= Hash::_side;
}

void Hash::xorCastlingRights(U64 &key, const CastlingRights rights)
{
	if (rights & CASTLE_WHITE_BOTH)
		key ^= _castlingRights[0];
	else if (rights & CASTLE_WHITE_KING)
		key ^= _castlingRights[1];
	else if (rights & CASTLE_WHITE_QUEEN)
		key ^= _castlingRights[2];

	if (rights & CASTLE_BLACK_BOTH)
		key ^= _castlingRights[3];
	else if (rights & CASTLE_BLACK_KING)
		key ^= _castlingRights[4];
	else if (rights & CASTLE_BLACK_QUEEN)
		key ^= _castlingRights[5];
}

void Hash::xorEnPassant(U64 &key, const byte square)
{
	if (square <= SQUARE_NB)
		key ^= _enPassant[col(square)];
}
