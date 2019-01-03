#pragma once

#include <unordered_set>

#include "Piece.h"
#include "../../StackVector.h"

namespace MoveGen
{
	template<std::size_t bufferSize>
	using PosVector = StackVector<Pos, bufferSize>;

	static PosVector<2> generatePawnAttacks(const Piece &piece, Pos pos);
	PosVector<4> generatePawnMoves(const Piece &piece, Pos pos, const Board &board);

	PosVector<8> generateKnightMoves(const Piece &piece, const Pos &pos, const Board &board);

	PosVector<13> generateBishopMoves(const Piece &piece, const Pos &pos, const Board &board);

	PosVector<14> generateRookMoves(const Piece &piece, const Pos &pos, const Board &board);

	PosVector<27> generateQueenMoves(const Piece &piece, const Pos &pos, const Board &board);

	std::unordered_set<Pos> getAllMovesPerColor(bool white, const Board &board);

	static PosVector<8> generateKingInitialMoves(Pos pos);
	PosVector<8> generateKingMoves(const Piece &piece, const Pos &pos, const Board &board);
}
