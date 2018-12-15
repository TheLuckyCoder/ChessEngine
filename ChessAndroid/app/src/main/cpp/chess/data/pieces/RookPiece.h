#pragma once

#include "Piece.h"

class RookPiece : public Piece
{
public:
	explicit RookPiece(const bool isWhite)
		: Piece(Type::ROOK, isWhite) {}

	explicit RookPiece(const Piece *piece)
		: Piece(piece) {}

private:
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override;
	int evaluatePiece(int x, int y) const override;
};
