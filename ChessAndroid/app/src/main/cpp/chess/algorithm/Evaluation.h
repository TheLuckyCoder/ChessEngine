#pragma once

#include "../data/Piece.h"
#include "../data/Score.h"
#include "../containers/Containers.h"

class Board;

class Evaluation final
{
public:
	constexpr static short PIECE_VALUE[] = { 0, 136, 782, 830, 1289, 2529, 0 };

	Evaluation() = delete;

	static int evaluate(const Board &board) noexcept;
	static short getPieceValue(Piece::Type type) noexcept
	{
		return PIECE_VALUE[toByte(type)];
	}

private:
	inline static Score evaluatePawn(const Piece &piece, const Pos &pos, const Board &board, const Attacks &ourAttacks, const Attacks &theirAttacks) noexcept;
	inline static Score evaluateKnight(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateBishop(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateRook(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateQueen(const Piece &piece, const Pos &pos, const Board &board) noexcept;
	inline static Score evaluateKing(const Piece &piece, const Pos &pos, const Board &board) noexcept;
};
