#include "KnightPiece.h"

#include "PieceEval.h"

void KnightPiece::calculateMoves(Pos & pos, std::vector<Pos>& moves, const Board & board) const
{
	const auto addPosIfValid = [&](const short x, const short y) {
		Pos startPos = Pos(pos.x + x, pos.y + y);

		if (startPos.isValid())
		{
			auto *other = board[startPos];

			if (!other || !hasSameColor(*other))
				moves.push_back(startPos);
		}
	};

	addPosIfValid(-1, 2);
	addPosIfValid(1, 2);

	addPosIfValid(-1, -2);
	addPosIfValid(1, -2);

	addPosIfValid(2, 1);
	addPosIfValid(2, -1);

	addPosIfValid(-2, 1);
	addPosIfValid(-2, -1);
}

int KnightPiece::evaluatePiece(const int x, const int y) const
{
	return PieceEval::KNIGHT + (isMaximising() ? PieceEval::KNIGHT_WHITE[x][y] : PieceEval::KNIGHT_BLACK[x][y]);
}
