#pragma once

#include <array>

#include "../Defs.h"
#include "../Piece.h"

class Board;

class Hash final
{
	using HashArray = std::array<std::array<std::array<u64, 7>, 2>, SQUARE_NB>;

	static HashArray _pieces;
	static u64 _side;
	static std::array<u64, 6> _castlingRights;
	static std::array<u64, 8> _enPassant;

public:
	Hash() = delete;
	Hash(const Hash &) = delete;
	Hash(Hash &&) = delete;
	~Hash() = delete;

	Hash &operator=(const Hash &) = delete;
	Hash &operator=(Hash &&) = delete;

	static void init();
	static u64 compute(const Board &board);

	static void makeMove(u64 &key, u8 selectedSq, u8 destSq, const Piece &selectedPiece,
						 const Piece &destPiece = EMPTY_PIECE);
	static void xorPiece(u64 &key, u8 sq, Piece piece);
	static void flipSide(u64 &key);
	static void xorCastlingRights(u64 &key, CastlingRights rights);
	static void xorEnPassant(u64 &key, u8 square);
};
