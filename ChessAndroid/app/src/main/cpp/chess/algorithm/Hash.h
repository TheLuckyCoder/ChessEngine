#pragma once

#include <array>

#include "../data/Defs.h"
#include "../data/Piece.h"

using U64 = std::uint64_t;

class Hash final
{
	using HashArray = std::array<std::array<std::array<U64, 7>, 2>, 64>;

	static HashArray s_Pieces;
	static U64 s_WhiteToMove;
	static std::array<U64, 4> s_CastlingRights;

public:
	Hash() = delete;
	Hash(const Hash&) = delete;
	Hash(Hash&&) = delete;

	Hash &operator=(const Hash&) = delete;
	Hash &operator=(Hash&&) = delete;

	static void init();
	static U64 compute(const Board &board);

	static void makeMove(U64 &key, byte selectedSq, byte destSq, const Piece &selectedPiece, const Piece &destPiece = Piece::EMPTY);
	static void promotePawn(U64 &key, byte sq, Color color, PieceType promotedType);
	static void xorPiece(U64 &key, byte sq, const Piece &piece);
	static void flipSide(U64 &key);
	static void xorCastlingRights(U64 &key, const CastlingRights rights);
};
