#pragma once

#include <unordered_set>

#include "Piece.h"

class KingPiece : public Piece
{
public:
	explicit KingPiece(bool isWhite)
		: Piece(Type::KING, isWhite) {}

	explicit KingPiece(const Piece *piece)
		: Piece(piece) {}

	std::vector<Pos> getInitialMoves(Pos pos) const;

private:
	std::unordered_set<Pos> getAllMovesPerColor(const Board &board, bool white) const;
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override;
	float evaluatePiece(const int x, const int y) const override;
};
