#pragma once

#include <unordered_map>

#include "../Game.h"
#include "../Pos.h"
#include "../Score.h"

class Piece;
class Board;

class Evaluation final
{
public:
	Evaluation() = delete;

	static int evaluate(const Board &board);

private:
	inline static Score evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, std::unordered_map<Pos, short> &opponentsAttacks);
	inline static Score evaluateKnight(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateBishop(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateRook(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateQueen(const Piece &piece, const Pos &pos, const Board &board);
	inline static Score evaluateKing(const Piece &piece, const Pos &pos, const Board &board, const std::unordered_map<Pos, short> &opponentsAttacks);
};
