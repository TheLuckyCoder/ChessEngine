#include "MoveOrdering.h"

#include <array>

#include "Evaluation.h"
#include "Search.h"
#include "../data/Board.h"

namespace MoveOrdering
{
	static constexpr int NORMAL_SCORE_BONUS = 1000000;
	static constexpr int PV_BONUS = NORMAL_SCORE_BONUS * 2;
	static constexpr int EN_PASSANT_BONUS = 105;

	static constexpr std::array VICTIM_SCORE = { 0, 100, 200, 300, 400, 500, 600 };
	static constexpr auto MVA_LVV = []
	{
		std::array<std::array<int, 7>, 7> array{};

		for (byte attacker = PAWN; attacker <= KING; ++attacker)
		{
			for (byte victim = PAWN; victim <= KING; ++victim)
			{
				array[victim][attacker] = VICTIM_SCORE[victim] + 6 - VICTIM_SCORE[attacker] / 100;
			}
		}

		return array;
	}();

	void sortMoves(const Board &board, MoveList &moveList) noexcept
	{
		const Move pvMove = Search::getTranspTable()[board.zKey].move;
		const auto &searchKillers = Search::getSearchKillers();
		const auto &searchHistory = Search::getSearchHistory();

		for (Move &move : moveList)
		{
			const auto flags = move.flags();

			if (move == pvMove)
				move.setScore(PV_BONUS);
			else if (flags & Move::Flag::CAPTURE)
				move.setScore(MVA_LVV[move.capturedPiece()][move.piece()] + NORMAL_SCORE_BONUS);
			else if (flags & Move::PROMOTION)
				move.setScore(Evaluation::getPieceValue(move.promotedPiece()) + NORMAL_SCORE_BONUS);
			else if (flags & Move::Flag::EN_PASSANT)
				move.setScore(EN_PASSANT_BONUS + NORMAL_SCORE_BONUS);
			else if (searchKillers[0][board.ply] == move.getContents())
				move.setScore(900000);
			else if (searchKillers[1][board.ply] == move.getContents())
				move.setScore(800000);
			else
				move.setScore(searchHistory[move.from()][move.to()]);
		}
	}

	void sortQMoves(MoveList &moveList) noexcept
	{
		for (Move &move : moveList)
		{
			const auto flags = move.flags();

			if (flags & Move::Flag::CAPTURE)
				move.setScore(MVA_LVV[move.capturedPiece()][move.piece()]);
			else if (flags & Move::PROMOTION)
				move.setScore(Evaluation::getPieceValue(move.promotedPiece()));
			else if (flags & Move::Flag::EN_PASSANT)
				move.setScore(EN_PASSANT_BONUS);
		}
	}

	Move getNextMove(MoveList &moveList) noexcept
	{
		Move *foundMove = moveList.begin();
		
		for (auto &&move : moveList)
		{
			if (move.getScore() > foundMove->getScore())
				foundMove = &move;
		}

		const Move bestMove = *foundMove;

		*foundMove = moveList.back();
		moveList.pop_back();

		return bestMove;
	}
}
