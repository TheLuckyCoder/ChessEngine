#pragma once

#include "../data/Piece.h"
#include "../data/Score.h"

class Board;

class Evaluation final
{
	constexpr static short PIECE_VALUE[] = { 0, 128, 781, 825, 1276, 2538, 0 };

public:
	Evaluation() = delete;
	Evaluation(const Evaluation&) = delete;
	Evaluation(Evaluation&&) = delete;

	static short evaluate(const Board &board) noexcept;
	static short getPieceValue(const PieceType type) noexcept
	{
		return PIECE_VALUE[type];
	}

private:
	static Score evaluatePawn(const Piece &piece, byte square, const Board &board) noexcept;
	inline static Score evaluateKnight(const Piece &piece, byte square, const Board &board) noexcept;
	static Score evaluateBishop(const Piece &piece, byte square, const Board &board) noexcept;
	static Score evaluateRook(const Piece &piece, byte square, const Board &board) noexcept;
	static Score evaluateQueen(const Piece &piece, const byte square, const Board &board) noexcept;
	inline static Score evaluateKing(const Piece &piece, const byte square, const Board &board) noexcept;
};
