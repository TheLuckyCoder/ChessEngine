#pragma once

#include <string>

#include "../Board.h"

class Evaluation final
{
public:
	static constexpr short TEMPO_BONUS = 20;

	static int value(const Board &board) noexcept;
	static int invertedValue(const Board &board) noexcept;
	static std::string traceValue(const Board &board);

	static constexpr short getPieceValue(const PieceType type) noexcept
	{
		constexpr short PieceValue[] = { 0, 128, 781, 825, 1276, 2538, 0 };

		return PieceValue[type];
	}
};
