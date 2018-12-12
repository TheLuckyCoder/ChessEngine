#pragma once

#include "Piece.h"

class PawnPiece : public Piece
{
public:
	explicit PawnPiece(bool isWhite)
		: Piece(Type::PAWN, isWhite) {}

	explicit PawnPiece(const Piece *piece)
		: Piece(piece) {}

	void getInitialMoves(Pos pos, std::vector<Pos> &moves, const Board &board) const;
	void addAttackMoves(Pos pos, std::vector<Pos> &moves) const;

private:
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override;
	int evaluatePiece(const int x, const int y) const override;
};
