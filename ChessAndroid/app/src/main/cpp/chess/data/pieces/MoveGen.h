#pragma once

#include <unordered_map>
#include <unordered_set>

#include "Piece.h"
#include "../../memory/StackVector.h"

template<std::size_t N>
using PosVector = StackVector<Pos, N>;

enum GenType : unsigned char
{
	CAPTURES,
	ATTACKS_DEFENSES,
	ALL
};

class MoveGen final
{
public:
	MoveGen() = delete;

	static PosVector<2> generatePawnAttacks(const Piece &piece, Pos pos);
	static PosVector<4> generatePawnMoves(const Piece &piece, Pos pos, const Board &board);

	static PosVector<8> generateKnightMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<13> generateBishopMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<14> generateRookMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<27> generateQueenMoves(const Piece &piece, const Pos &pos, const Board &board);

	static PosVector<8> generateKingInitialMoves(Pos pos);
	static PosVector<8> generateKingMoves(const Piece &piece, const Pos &pos, const Board &board);

	static std::unordered_set<Pos> getAllAttacksPerColor(bool white, const Board &board);
	static std::unordered_map<Pos, short> getAllMovesPerColor(bool white, const Board &board);
};
