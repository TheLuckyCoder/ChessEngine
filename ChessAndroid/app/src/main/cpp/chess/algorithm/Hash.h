#pragma once

#include <array>

#include "../data/Pos.h"
#include "../data/Piece.h"

using U64 = std::uint64_t;

class Hash final
{
private:
	using HashArray = std::array<std::array<std::array<U64, 12>, 8>, 8>;
	inline static bool initialized = false;

	static HashArray array;
	static U64 whiteToMove;
public:
	static std::array<U64, 4> castlingRights;

	Hash() = delete;

	static void init();
	static U64 getHash(const Pos &pos, const Piece &piece);
	static U64 compute(const Board &board);

	static void makeMove(U64 &key, const Pos &selectedPos, const Pos &destPos, const Piece &selectedPiece, const Piece &destPiece);
	static void promotePawn(U64 &key, const Pos &startPos, const Pos &destPos, bool isWhite, Type promotedType);
	static void flipSide(U64 &key);

private:
	static byte indexOf(const Piece &piece);
};
