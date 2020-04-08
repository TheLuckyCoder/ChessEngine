#pragma once

#include <algorithm>
#include <array>

#include "Evaluation.h"
#include "../data/Move.h"

namespace MoveOrdering
{
	static constexpr std::array VictimScore = { 0, 100, 200, 300, 400, 500, 600 };
	
	constexpr auto MvaLvv = []
	{
		std::array<std::array<int, 7>, 7> array{};

		for (int attacker = PAWN; attacker <= KING; ++attacker)
		{
			for (int victim = PAWN; victim <= KING; ++victim)
			{
				array[victim][attacker] = VictimScore[victim] + 6 - VictimScore[attacker] / 100;
			}
		}

		return array;
	}();

	inline void sortMoves(const Board &board, Move *begin, Move *end) noexcept
	{
		constexpr int NORMAL_SCORE_BONUS = 1000000;
		constexpr int PV_BONUS = NORMAL_SCORE_BONUS * 2;
		constexpr int EN_PASSANT_BONUS = NORMAL_SCORE_BONUS + 105;
		
		const Move pvMove = Search::getTranspTable()[board.zKey].move;
		auto &searchKillers = Search::getSearchKillers();
		auto &searchHistory = Search::getSearchHistory();
		
		for (Move *it = begin; it != end; ++it)
		{
			Move &move = *it;
			const auto flags = move.flags(); 
			
			if (move == pvMove)
				move.setScore(PV_BONUS);
			else if (flags & Move::Flag::CAPTURE)
				move.setScore(MvaLvv[move.capturedPiece()][move.piece()] + NORMAL_SCORE_BONUS);
			else if (flags & Move::PROMOTION)
				move.setScore(Evaluation::getPieceValue(move.promotedPiece()) + NORMAL_SCORE_BONUS);
			else if (flags & Move::Flag::EN_PASSANT)
				move.setScore(EN_PASSANT_BONUS);
			else if (searchKillers[0][board.ply] == move)
					move.setScore(900000);
			else if (searchKillers[1][board.ply] == move)
				move.setScore(800000);
			else
				move.setScore(searchHistory[move.from()][move.to()]);
		}
		
		std::sort(begin, end, std::greater<>());
	}
}
