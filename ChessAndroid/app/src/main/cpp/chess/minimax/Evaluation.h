#pragma once

#include "../data/Pos.h"
#include "../data/Score.h"
#include "../containers/Containers.h"

class Piece;
class Board;

class Evaluation final
{
public:
	Evaluation() = delete;

	static int evaluate(const Board &board);

private:
	inline static Score evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, PosMap &opponentsAttacks);
	inline static Score evaluateKnight(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateBishop(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateRook(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateQueen(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateKing(const Piece &piece, const Pos &pos, const Board &board);
};
