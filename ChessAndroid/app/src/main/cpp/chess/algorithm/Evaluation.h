#pragma once

#include <array>

#include "../data/Board.h"
#include "../data/Score.h"
#include "../containers/PawnStructureTable.h"

class Evaluation final
{
	static constexpr short _PIECE_VALUE[] = { 0, 128, 781, 825, 1276, 2538, 0 };
	static PawnStructureTable _pawnTable;

public:
	static constexpr short TEMPO_BONUS = 20;

	struct Result
	{
		const Board &board;
		short value{};

		short invertedValue() const noexcept { return board.colorToMove ? value : -value; }
	};
	
	static Result evaluate(const Board &board) noexcept;
	
	static constexpr short getPieceValue(const PieceType type) noexcept
	{
		return _PIECE_VALUE[type];
	}

private:
	explicit Evaluation(const Board &board);
	
	template <Color Us>
	Score evaluatePieces() noexcept;
	template <Color Us>
	Score evaluateAttacks() const noexcept;
	template <Color Us>
	Score evaluatePawn(byte square) const noexcept;
	template <Color Us>
	Score evaluateKnight(byte square) const noexcept;
	template <Color Us>
	Score evaluateBishop(byte square) const noexcept;
	template <Color Us>
	Score evaluateRook(byte square) const noexcept;
	template <Color Us>
	Score evaluateQueen(byte square) const noexcept;
	template <Color Us>
	Score evaluateKing(byte square) const noexcept;

	const Board &board;
	std::array<std::array<U64, 7>, 2> _attacks{};
	std::array<U64, 2> _attacksMultiple{};
	std::array<U64, 2> _attacksAll{};
	std::array<U64, 2> _mobilityArea{};
};
