#pragma once

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
	MoveGen(const MoveGen&) = delete;
	MoveGen(MoveGen&&) = delete;

	static U64 generatePawnMoves(const Piece &piece, byte square, const Board &board);
	static U64 generateKnightMoves(const Piece &piece, byte square, const Board &board);
	static U64 generateBishopMoves(const Piece &piece, byte square, const Board &board);
	static U64 generateRookMoves(const Piece &piece, byte square, const Board &board);
	static U64 generateQueenMoves(const Piece &piece, byte square, const Board &board);
	static U64 generateKingMoves(const Piece &piece, byte square, const Board &board);
};

#include "MoveGen.inl"
