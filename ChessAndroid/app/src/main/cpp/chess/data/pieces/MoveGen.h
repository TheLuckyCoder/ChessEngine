#pragma once

#include <vector>
#include <unordered_set>

#include "Piece.h"

namespace MoveGen
{
	static void generatePawnAttacks(const Piece &piece, Pos pos, std::vector<Pos> &moves);
	void generatePawnMoves(const Piece &piece, Pos pos, std::vector<Pos> &moves, const Board &board);

	void generateKnightMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board);

	void generateBishopMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board);

	void generateRookMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board);

	void generateQueenMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board);

	std::unordered_set<Pos> getAllMovesPerColor(bool white, const Board &board);
	static std::vector<Pos> generateKingInitialMoves(Pos pos);
	void generateKingMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board);
}
