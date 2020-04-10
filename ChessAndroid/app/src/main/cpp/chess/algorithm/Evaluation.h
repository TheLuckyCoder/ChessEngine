#pragma once

#include "../data/Score.h"
#include "../containers/PawnStructureTable.h"

class Board;

class Evaluation final
{
	static constexpr short PIECE_VALUE[] = { 0, 128, 781, 825, 1276, 2538, 0 };
	static PawnStructureTable _pawnTable;

public:
	Evaluation() = delete;
	Evaluation(const Evaluation&) = delete;
	Evaluation(Evaluation&&) = delete;

	Evaluation &operator=(const Evaluation&) = delete;
	Evaluation &operator=(Evaluation&&) = delete;

	static short evaluate(const Board &board) noexcept;
	template <Color Us>
	static Score evaluatePieces(const Board &board) noexcept;
	static short getPieceValue(const PieceType type) noexcept
	{
		return PIECE_VALUE[type];
	}
	static PawnStructureTable &getPawnTable() noexcept
	{
		return _pawnTable;
	}

private:
	template <Color Us>
	static Score evaluatePawn(const Board &board, byte square) noexcept;
	
	template <Color Us>
	static Score evaluateKnight(const Board &board, byte square) noexcept;
	
	template <Color Us>
	static Score evaluateBishop(const Board &board, byte square) noexcept;
	
	template <Color Us>
	static Score evaluateRook(const Board &board, byte square) noexcept;
	
	template <Color Us>
	static Score evaluateQueen(const Board &board, byte square) noexcept;
	
	template <Color Us>
	static Score evaluateKing(const Board &board, byte square) noexcept;
};
