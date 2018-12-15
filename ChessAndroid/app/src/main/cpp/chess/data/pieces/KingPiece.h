#pragma once

#include <unordered_set>

#include "Piece.h"

class KingPiece : public Piece
{
public:
	explicit KingPiece(const bool isWhite)
		: Piece(Type::KING, isWhite) {}

	explicit KingPiece(const Piece *piece)
		: Piece(piece) {}

	static std::vector<Pos> getInitialMoves(Pos pos);

private:
	std::unordered_set<Pos> getAllMovesPerColor(const Board &board, bool white) const;
	void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const override;
	int evaluatePiece(int x, int y) const override;
};
