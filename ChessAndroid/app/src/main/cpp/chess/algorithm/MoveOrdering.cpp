#include "MoveOrdering.h"

#include "Evaluation.h"
#include "../Board.h"

namespace MoveOrdering
{
	static constexpr int EN_PASSANT_SCORE = 105;

	static constexpr std::array VICTIM_SCORE = { 0, 100, 200, 300, 400, 500, 600 };
	static constexpr auto MVA_LVV = []
	{
		std::array<std::array<int, 7>, 7> array{};

		for (u8 attacker = PAWN; attacker <= KING; ++attacker)
		{
			for (u8 victim = PAWN; victim <= KING; ++victim)
			{
				array[victim][attacker] = VICTIM_SCORE[victim] + 6 - VICTIM_SCORE[attacker] / 100;
			}
		}

		return array;
	}();

	void sortMoves(const Thread &thread, const Board &board, MoveList &moveList) noexcept
	{
		const auto probeResult = Search::getTranspTable().probe(board.zKey);
		const Move pvMove = probeResult.has_value() ? probeResult->move() : Move{};

		for (Move &move : moveList)
		{
			const auto flags = move.flags();

			if (move.fromToBits() == pvMove.fromToBits())
				move.setScore(PV_SCORE);
			else if (flags.capture())
				move.setScore(MVA_LVV[move.capturedPiece()][move.piece()] + NORMAL_SCORE);
			else if (flags.promotion())
				move.setScore(Evaluation::getPieceValue(move.promotedPiece()) + NORMAL_SCORE);
			else if (flags.enPassant())
				move.setScore(EN_PASSANT_SCORE + NORMAL_SCORE);
			else if (thread.killers[0][board.ply] == move.getContents())
				move.setScore(900000);
			else if (thread.killers[1][board.ply] == move.getContents())
				move.setScore(800000);
			else
				move.setScore(thread.history[move.from()][move.to()]);
		}
	}

	void sortQMoves(MoveList &moveList) noexcept
	{
		for (Move &move : moveList)
		{
			const auto flags = move.flags();

			if (flags.capture())
				move.setScore(MVA_LVV[move.capturedPiece()][move.piece()]);
			else if (flags.promotion())
				move.setScore(Evaluation::getPieceValue(move.promotedPiece()));
			else if (flags.enPassant())
				move.setScore(EN_PASSANT_SCORE);
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
