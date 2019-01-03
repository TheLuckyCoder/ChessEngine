#include "MoveGen.h"

#include <algorithm>
#include <iterator>
#include <unordered_set>

#include "../../BoardManager.h"

namespace MoveGen
{

	PosVector<2> generatePawnAttacks(const Piece &piece, Pos pos)
	{
		PosVector<2> moves;

		piece.isWhite ? pos.y++ : pos.y--;

		piece.isWhite ? pos.x-- : pos.x++;
		if (pos.isValid())
			moves.push_back(pos);

		pos.x += piece.isWhite ? 2 : -2;
		if (pos.isValid())
			moves.push_back(pos);

		return moves;
	}

	PosVector<4> generatePawnMoves(const Piece &piece, Pos pos, const Board &board)
	{
		PosVector<4> moves;

		piece.isWhite ? pos.y++ : pos.y--;
		if (!board[pos])
		{
			moves.push_back(pos);

			if (!piece.hasBeenMoved) {
				Pos posCopy = pos;

				piece.isWhite ? posCopy.y++ : posCopy.y--;
				if (!board[posCopy])
					moves.push_back(posCopy);
			}
		}

		piece.isWhite ? pos.x-- : pos.x++;
		if (pos.isValid())
		{
			const auto &other = board[pos];

			if (other && !piece.hasSameColor(other))
				moves.push_back(pos);
		}

		pos.x += piece.isWhite ? 2 : -2;
		if (pos.isValid())
		{
			auto &other = board[pos];

			if (other && !piece.hasSameColor(other))
				moves.push_back(pos);
		}

		return moves;
	}

	PosVector<8> generateKnightMoves(const Piece &piece, const Pos &pos, const Board &board)
	{
		PosVector<8> moves;

		const auto addPosIfValid = [&](const short x, const short y) {
			Pos startPos(pos.x + x, pos.y + y);

			if (startPos.isValid())
			{
				auto &other = board[startPos];

				if (!other || !piece.hasSameColor(other))
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

		return moves;
	}

	PosVector<13> generateBishopMoves(const Piece &piece, const Pos &pos, const Board &board)
	{
		PosVector<13> moves;
		Pos posCopy = pos;

		while (posCopy.x < 7 && posCopy.y > 0)
		{
			posCopy.x++;
			posCopy.y--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x < 7 && posCopy.y < 7)
		{
			posCopy.x++;
			posCopy.y++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x > 0 && posCopy.y > 0)
		{
			posCopy.x--;
			posCopy.y--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x > 0 && posCopy.y < 7)
		{
			posCopy.x--;
			posCopy.y++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		return moves;
	}

	PosVector<14> generateRookMoves(const Piece &piece, const Pos &pos, const Board &board)
	{
		PosVector<14> moves;
		Pos posCopy = pos;

		while (posCopy.x > 0)
		{
			posCopy.x--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x < 7)
		{
			posCopy.x++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.y > 0)
		{
			posCopy.y--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.y < 7)
		{
			posCopy.y++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		return moves;
	}

	PosVector<27> generateQueenMoves(const Piece &piece, const Pos &pos, const Board &board)
	{
		PosVector<27> moves;
		Pos posCopy = pos;

		// Vertical and Horizontal
		while (posCopy.x > 0)
		{
			posCopy.x--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x < 7)
		{
			posCopy.x++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.y > 0)
		{
			posCopy.y--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.y < 7)
		{
			posCopy.y++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		// Diagonal
		posCopy = pos;
		while (posCopy.x < 7 && posCopy.y > 0)
		{
			posCopy.x++;
			posCopy.y--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x < 7 && posCopy.y < 7)
		{
			posCopy.x++;
			posCopy.y++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x > 0 && posCopy.y > 0)
		{
			posCopy.x--;
			posCopy.y--;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		posCopy = pos;
		while (posCopy.x > 0 && posCopy.y < 7)
		{
			posCopy.x--;
			posCopy.y++;

			auto &other = board[posCopy];

			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		return moves;
	}

	std::unordered_set<Pos> getAllMovesPerColor(const bool white, const Board &board)
	{
		std::unordered_set<Pos> allMoves;
		allMoves.reserve(64);

		for (byte i = 0; i < 8; i++)
			for (byte j = 0; j < 8; j++)
			{
				auto &piece = board.data[i][j];
				if (piece && piece.isWhite == white)
				{
					if (piece.type == Piece::Type::PAWN)
					{
						auto moves = generatePawnAttacks(piece, Pos(i, j));
						std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
					}
					else if (piece.type == Piece::Type::KING)
					{
						auto moves = generateKingInitialMoves(Pos(i, j));
						std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
					}
					else
					{
						auto moves = piece.getPossibleMoves(Pos(i, j), board);
						std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
					}
				}
			}

		return allMoves;
	}

	PosVector<8> generateKingInitialMoves(Pos pos)
	{
		PosVector<8> moves;

		// Vertical and Horizontal
		if (pos.x > 0)
			moves.emplace_back(pos.x - 1, pos.y);

		if (pos.x < 7)
			moves.emplace_back(pos.x + 1, pos.y);

		if (pos.y > 0)
			moves.emplace_back(pos.x, pos.y - 1);

		if (pos.y < 7)
			moves.emplace_back(pos.x, pos.y + 1);

		// Diagonal
		if (pos.x < 7 && pos.y > 0)
			moves.emplace_back(pos.x + 1, pos.y - 1);

		if (pos.x < 7 && pos.y < 7)
			moves.emplace_back(pos.x + 1, pos.y + 1);

		if (pos.x > 0 && pos.y > 0)
			moves.emplace_back(pos.x - 1, pos.y - 1);

		if (pos.x > 0 && pos.y < 7)
			moves.emplace_back(pos.x - 1, pos.y + 1);

		return moves;
	}

	PosVector<8> generateKingMoves(const Piece &piece, const Pos &pos, const Board &board)
	{
		auto moves = generateKingInitialMoves(pos);

		{
			const auto iterator = std::remove_if(moves.begin(), moves.end(),
				[&piece, &board](const Pos &pos) { return board[pos] && piece.hasSameColor(board[pos]); });
			moves.erase(iterator, moves.end());
		}

		if (moves.empty()) return moves;

		const auto opponentsMoves = getAllMovesPerColor(!piece.isWhite, board);

		for (unsigned int i = 0; i < moves.size(); i++)
		{
			if (std::find(opponentsMoves.begin(), opponentsMoves.end(), moves[i]) != opponentsMoves.end())
			{
				moves[i] = moves.back();
				moves.pop_back();
				i--;
			}
		}

		// Castling
		if (!piece.hasBeenMoved && std::find(opponentsMoves.begin(), opponentsMoves.end(), pos) == opponentsMoves.end())
		{
			Pos posCopy = pos;
			while (posCopy.x < 7)
			{
				posCopy.x++;
				auto &other = board[posCopy];

				if (std::find(opponentsMoves.begin(), opponentsMoves.end(), posCopy) != opponentsMoves.end())
					break;

				if (posCopy.x < 7)
				{
					if (other)
						break;
				}
				else if (other && piece.hasSameColor(other) && other.type == Piece::Type::ROOK && !other.hasBeenMoved)
					moves.emplace_back(6, posCopy.y);
			}

			posCopy = pos;
			while (posCopy.x > 0)
			{
				posCopy.x--;
				const auto &other = board[posCopy];

				if ((posCopy.x == 2 || posCopy.x == 3) &&
					std::find(opponentsMoves.begin(), opponentsMoves.end(), posCopy) != opponentsMoves.end())
					break;

				if (posCopy.x > 0)
				{
					if (other)
						break;
				}
				else if (other && piece.hasSameColor(other) && other.type == Piece::Type::ROOK && !other.hasBeenMoved)
					moves.emplace_back(2, posCopy.y);
			}
		}

		return moves;
	}

}
