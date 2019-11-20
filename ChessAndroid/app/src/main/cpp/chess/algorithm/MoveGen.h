#pragma once

#include "../containers/Containers.h"

enum GenType : unsigned char
{
	ALL,
	CAPTURES,
	ATTACKS_DEFENSES,
	KING_DANGER
};

template<GenType T, bool ToList = true>
class MoveGen final
{
public:
	MoveGen() = delete;
	MoveGen(const MoveGen&) = delete;
	MoveGen(MoveGen&&) = delete;

	static auto generatePawnMoves(const Piece &piece, byte square, const Board &board);
	static auto generateKnightMoves(const Piece &piece, byte square, const Board &board);
	static auto generateBishopMoves(const Piece &piece, byte square, const Board &board);
	static auto generateRookMoves(const Piece &piece, byte square, const Board &board);
	static auto generateQueenMoves(const Piece &piece, byte square, const Board &board);
	static auto generateKingMoves(const Piece &piece, byte square, const Board &board);

	template <class Func>
	static void forEachAttack(bool white, const Board &board, Func &&func);
	static AttacksMap getAttacksPerColor(bool white, const Board &board);
};

#include "MoveGen.inl"
