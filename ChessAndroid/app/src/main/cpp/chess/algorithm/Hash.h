#pragma once

#include <array>

#include "../Defs.h"
#include "../Piece.h"

class Board;

class Hash final
{
	using HashArray = std::array<std::array<std::array<U64, 7>, 2>, SQUARE_NB>;

	static HashArray _pieces;
	static U64 _side;
	static std::array<U64, 6> _castlingRights;
	static std::array<U64, 8> _enPassant;

public:
	Hash() = delete;
	Hash(const Hash &) = delete;
	Hash(Hash &&) = delete;
	~Hash() = delete;

	Hash &operator=(const Hash &) = delete;
	Hash &operator=(Hash &&) = delete;

	static void init();
	static U64 compute(const Board &board);

	static void makeMove(U64 &key, byte selectedSq, byte destSq, const Piece &selectedPiece,
						 const Piece &destPiece = EMPTY_PIECE);
	static void xorPiece(U64 &key, byte sq, Piece piece);
	static void flipSide(U64 &key);
	static void xorCastlingRights(U64 &key, CastlingRights rights);
	static void xorEnPassant(U64 &key, byte square);
};
