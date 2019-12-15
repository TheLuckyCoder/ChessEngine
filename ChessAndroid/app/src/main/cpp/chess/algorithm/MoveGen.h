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

	static U64 getPawnMoves(Color color, byte square, const Board &board) noexcept;
	static U64 getKnightMoves(Color color, byte square, const Board &board) noexcept;
	static U64 getBishopMoves(Color color, byte square, const Board &board) noexcept;
	static U64 getRookMoves(Color color, byte square, const Board &board) noexcept;
	static U64 getQueenMoves(Color color, byte square, const Board &board) noexcept;
	static U64 getKingMoves(Color color, byte square, const Board &board) noexcept;
};

#include "MoveGen.inl"
