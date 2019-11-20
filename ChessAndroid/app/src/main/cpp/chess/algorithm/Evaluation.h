#pragma once

#include "../data/Piece.h"
#include "../data/Score.h"
#include "../containers/Containers.h"

class Board;

class Evaluation final
{
	constexpr static short PIECE_VALUE[] = { 0, 136, 782, 830, 1289, 2529, 0 };

public:
	Evaluation() = delete;
	Evaluation(const Evaluation&) = delete;
	Evaluation(Evaluation&&) = delete;

	static short simpleEvaluation(const Board &board) noexcept;
	static short evaluate(const Board &board) noexcept;
	static short getPieceValue(const PieceType type) noexcept
	{
		return PIECE_VALUE[type];
	}

private:
	static Score evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, const AttacksMap &ourAttacks, const AttacksMap &theirAttacks) noexcept;
	inline static Score evaluateKnight(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	static Score evaluateBishop(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	static Score evaluateRook(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	static Score evaluateQueen(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateKing(const Piece &piece, const Pos &pos, const Board &board) noexcept;
};
