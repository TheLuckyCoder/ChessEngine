#pragma once

#include "Piece.h"

class KnightPiece : public Piece
{
public:
	explicit KnightPiece(const bool isWhite)
		: Piece(Type::KNIGHT, isWhite) {}

	explicit KnightPiece(const Piece *piece)
		: Piece(piece) {}

private:
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override;
	int evaluatePiece(int x, int y) const override;
};
