#pragma once

#include "../containers/Containers.h"

enum GenType : unsigned char
{
	ALL,
	CAPTURES,
	ATTACKS_DEFENSES
};

template<GenType T>
class MoveGen final
{
public:
	MoveGen() = delete;

	static PosVector<4> generatePawnMoves(const Piece &piece, Pos pos, const Board &board);

	static PosVector<8> generateKnightMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<13> generateBishopMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<14> generateRookMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<27> generateQueenMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<8> generateKingInitialMoves(Pos pos);
	static PosVector<8> generateKingMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosSet getAttacksPerColor(bool white, const Board &board);
	static PosMap getMovesPerColorMap(bool white, const Board &board);
};

#include "MoveGen.inl"
