#include "KingPiece.h"

#include <algorithm>
#include <iterator>

#include "Pieces.h"
#include "../../BoardManager.h"

std::vector<Pos> KingPiece::getInitialMoves(Pos pos) const
{
	std::vector<Pos> moves;
	moves.reserve(8);

	Pos posCopy = pos;

	// Vertical and Horizontal
	if (posCopy.x > 0)
	{
		posCopy.x--;
		moves.push_back(posCopy);

		posCopy = pos;
	}

	if (posCopy.x < 7)
	{
		posCopy.x++;
		moves.push_back(posCopy);

		posCopy = pos;
	}

	if (posCopy.y > 0)
	{
		posCopy.y--;
		moves.push_back(posCopy);

		posCopy = pos;
	}

	if (posCopy.y < 7)
	{
		posCopy.y++;
		moves.push_back(posCopy);

		posCopy = pos;
	}

	// Diagonal
	if (posCopy.x < 7 && posCopy.y > 0)
	{
		posCopy.x++;
		posCopy.y--;
		moves.push_back(posCopy);

		posCopy = pos;
	}

	if (posCopy.x < 7 && posCopy.y < 7)
	{
		posCopy.x++;
		posCopy.y++;
		moves.push_back(posCopy);

		posCopy = pos;
	}

	if (posCopy.x > 0 && posCopy.y > 0)
	{
		posCopy.x--;
		posCopy.y--;
		moves.push_back(posCopy);

		posCopy = pos;
	}

	if (posCopy.x > 0 && posCopy.y < 7)
	{
		posCopy.x--;
		posCopy.y++;
		moves.push_back(posCopy);
	}

	return moves;
}

std::unordered_set<Pos> KingPiece::getAllMovesPerColor(const Board &board, bool white) const
{
	std::unordered_set<Pos> allMoves;
	allMoves.reserve(32);

	for (short i = 0; i < 8; i++)
		for (short j = 0; j < 8; j++)
		{
			Piece *piece = board.data[i][j];
			if (piece && piece->isWhite == white)
			{
				if (piece->type == Piece::Type::PAWN)
				{
					auto *pawn = static_cast<PawnPiece*>(piece);
					std::vector<Pos> moves;
					pawn->addAttackMoves(Pos(i, j), moves);

					std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
				}
				else if (piece->type == Piece::Type::KING)
				{
					auto moves = static_cast<KingPiece*>(piece)->getInitialMoves(Pos(i, j));
					std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
				}
				else
				{
					auto moves = piece->getPossibleMoves(Pos(i, j), board);
					std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
				}
			}
		}

	return allMoves;
}

void KingPiece::calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const
{
	auto initialMoves = getInitialMoves(pos);

	{
		auto iter = std::remove_if(initialMoves.begin(), initialMoves.end(),
			[*this, &board](const Pos &pos) { return board[pos] && hasSameColor(*board[pos]); });
		initialMoves.erase(iter, initialMoves.end());
	}

	if (initialMoves.empty()) return;
	std::move(initialMoves.begin(), initialMoves.end(), std::back_inserter(moves));

	// Remove the possible moves of the opponent's pieces
	std::unordered_set<Pos> opponentsMoves = getAllMovesPerColor(board, !isWhite);

	for (unsigned int i = 0; i < moves.size(); i++)
	{
		for (auto it = opponentsMoves.begin(); it != opponentsMoves.end(); it++)
		{
			if (moves[i] == *it)
			{
				moves[i] = std::move(moves[moves.size() - 1]);
				moves.pop_back();
				break;
			}
		}
	}

	// Castling
	if (!hasBeenMoved)
	{
		Pos posCopy = pos;
		while (posCopy.x < 7)
		{
			posCopy.x++;
			auto *other = board[posCopy];

			if (std::find(opponentsMoves.begin(), opponentsMoves.end(), posCopy) != opponentsMoves.end())
				break;

			if (posCopy.x < 7)
			{
				if (other)
					break;
			}
			else if (other && hasSameColor(*other) && other->type == Type::ROOK)
			{
				if (!static_cast<const RookPiece*>(other)->hasBeenMoved)
					moves.emplace_back(6, posCopy.y);
			}
		}

		posCopy = pos;
		while (posCopy.x > 0)
		{
			posCopy.x--;
			auto *other = board[posCopy];

			if (std::find(opponentsMoves.begin(), opponentsMoves.end(), posCopy) != opponentsMoves.end())
				break;

			if (posCopy.x > 0)
			{
				if (other)
					break;
			}
			else if (other && hasSameColor(*other) && other->type == Type::ROOK)
			{
				if (!static_cast<const RookPiece*>(other)->hasBeenMoved)
					moves.emplace_back(2, posCopy.y);
			}
		}
	}
}

float KingPiece::evaluatePiece(const int x, const int y) const
{
	return PieceEval::KING + (isMaximising() ? PieceEval::KING_WHITE[x][y] : PieceEval::KING_BLACK[x][y]);
}
