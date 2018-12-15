#pragma once

#include "Piece.h"

class QueenPiece : public Piece
{
public:
	explicit QueenPiece(const bool isWhite)
		: Piece(Type::QUEEN, isWhite) {}

	explicit QueenPiece(const Piece *piece)
		: Piece(piece) {}

private:
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override;
	int evaluatePiece(int x, int y) const override;
};
