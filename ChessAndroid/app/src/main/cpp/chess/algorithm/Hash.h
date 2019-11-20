#pragma once

#include <array>

#include "../data/Enums.h"
#include "../data/Piece.h"

using U64 = std::uint64_t;

class Hash final
{
private:
	using HashArray = std::array<std::array<U64, 12>, 64>;

	static HashArray s_Pieces;
	static U64 s_WhiteToMove;
	static std::array<U64, 4> s_CastlingRights;

public:
	Hash() = delete;
	Hash(const Hash&) = delete;
	Hash(Hash&&) = delete;

	static void init();
	static U64 movePiece(byte sq, const Piece &piece);
	static U64 compute(const Board &board);

	static void makeMove(U64 &key, byte selectedSq, byte destSq, const Piece &selectedPiece, const Piece &destPiece = Piece::EMPTY);
	static void promotePawn(U64 &key, byte sq, Color color, PieceType promotedType);
	static void xorPiece(U64 &key, byte sq, const Piece &piece);
	static void flipSide(U64 &key);
	static void xorCastlingRights(U64 &key, CastlingRights rights);

private:
	static byte indexOf(const Piece &piece);
};
