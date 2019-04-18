#pragma once

#include <array>

#include "../data/Piece.h"
#include "Evaluation.h"
#include "../data/Board.h"

class MoveOrdering
{
	using ScoresArray = std::array<std::array<int, 7>, 7>;

	constexpr static int VICTIM_SCORES[] = { 0, 100, 200, 300, 400, 500, 600 };
	inline static ScoresArray MVV_LVA_SCORES{};

public:
	MoveOrdering() = delete;

	static void init() noexcept
	{
		for (byte attacker = Type::PAWN; attacker <= Type::KING; ++attacker)
			for (byte victim = Type::PAWN; victim <= Type::KING; ++victim)
				MVV_LVA_SCORES[victim][attacker] = VICTIM_SCORES[victim] + 6 - (VICTIM_SCORES[attacker] / 100);
	}

	static int getCaptureScore(Type victim, Type attacker) noexcept
	{
		return MoveOrdering::MVV_LVA_SCORES[victim][attacker] + 1000000;
	}

	static int getQuietMoveScore(const Board &board, bool white) noexcept
	{
		/*if (searchKillers[0][depth] == move)
			return 900000;
		if (searchKillers[1][depth] == move)
			return 80000;*/
		const int evaluation = Evaluation::simpleEvaluation(board);
		return white ? evaluation : -evaluation;
	}

	static int getEnPassantScore() noexcept
	{
		return 105 + 1000000;
	}
};
