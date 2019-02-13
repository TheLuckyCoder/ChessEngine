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

	static int evaluate(const Board &board) noexcept;

private:
	inline static Score evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, const PosMap &opponentsAttacks) noexcept;
	inline static Score evaluateKnight(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateBishop(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateRook(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateQueen(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateKing(const Piece &piece, const Pos &pos, const Board &board) noexcept;
};
