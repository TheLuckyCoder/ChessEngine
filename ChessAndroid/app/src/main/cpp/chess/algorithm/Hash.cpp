#include "Hash.h"

#include <random>

#include "../data/Board.h"

Hash::HashArray Hash::_pieces{};
U64 Hash::_side;
std::array<U64, 4> Hash::_castlingRights;
std::array<U64, SQUARE_NB> Hash::_enPassant;

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

	for (byte i = 0; i < SQUARE_NB; ++i)
		if (const Piece &piece = board.getPiece(i); piece)
			hash ^= _pieces[i][piece.color()][piece.type()];

	xorCastlingRights(hash, static_cast<CastlingRights>(board.castlingRights));

	if (board.enPassantSq != SQ_NONE)
		xorEnPassant(hash, board.enPassantSq);

	if (board.colorToMove)
		flipSide(hash);

	return hash;
}

void Hash::makeMove(U64 &key, const byte selectedSq, const byte destSq, const Piece &selectedPiece, const Piece &destPiece)
{
	// Remove Selected Piece
	xorPiece(key, selectedSq, selectedPiece);

	if (destPiece) // Remove Destination Piece if any
		xorPiece(key, destSq, destPiece);

	// Add Selected Piece to Destination
	xorPiece(key, destSq, selectedPiece);
}

void Hash::promotePawn(U64 &key, const byte sq, const Color color, const PieceType promotedType)
{
	// Remove the Pawn
	key ^= _pieces[sq][color][PAWN];

	// Add the Promoted Piece
	key ^= _pieces[sq][color][promotedType];
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
	if (rights & CASTLE_WHITE_KING)
		key ^= _castlingRights[0];
	if (rights & CASTLE_WHITE_QUEEN)
		key ^= _castlingRights[1];

	if (rights & CASTLE_BLACK_KING)
		key ^= _castlingRights[2];
	if (rights & CASTLE_BLACK_QUEEN)
		key ^= _castlingRights[3];
}

void Hash::xorEnPassant(U64 &key, const byte square)
{
	if (square != SQ_NONE)
		key ^= _enPassant[square];
}
