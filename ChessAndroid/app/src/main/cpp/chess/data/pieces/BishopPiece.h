#pragma once

#include "Piece.h"

class BishopPiece : public Piece
{
public:
	explicit BishopPiece(bool isWhite)
		: Piece(Type::BISHOP, isWhite) {}

	explicit BishopPiece(const Piece *piece)
		: Piece(piece) {}

private:
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override;
    int evaluatePiece(int x, int y) const override;
};
