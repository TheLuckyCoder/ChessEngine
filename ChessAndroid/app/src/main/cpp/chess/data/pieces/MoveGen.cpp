#include "MoveGen.h"

#include <algorithm>
#include <iterator>
#include <unordered_set>

#include "../../BoardManager.h"

namespace MoveGen
{

	void generatePawnAttacks(const Piece &piece, Pos pos, std::vector<Pos> &moves)
	{
		piece.isWhite ? pos.y++ : pos.y--;

		piece.isWhite ? pos.x-- : pos.x++;
		if (pos.isValid())
			moves.push_back(pos);

		pos.x += piece.isWhite ? 2 : -2;
		if (pos.isValid())
			moves.push_back(pos);
	}

	void generatePawnMoves(const Piece &piece, Pos pos, std::vector<Pos> &moves, const Board &board)
	{
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
	}

	void generateKnightMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board)
	{
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
	}

	void generateBishopMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board)
	{
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
	}

	void generateRookMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board)
	{
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
	}

	void generateQueenMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board)
	{
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
	}

	std::unordered_set<Pos> getAllMovesPerColor(const bool white, const Board &board)
	{
		std::unordered_set<Pos> allMoves;
		allMoves.reserve(64);

		for (short i = 0; i < 8; i++)
			for (short j = 0; j < 8; j++)
			{
				auto &piece = board.data[i][j];
				if (piece && piece.isWhite == white)
				{
					if (piece.type == Piece::Type::PAWN)
					{
						std::vector<Pos> moves;
						generatePawnAttacks(piece, Pos(i, j), moves);

						std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
					}
					else if (piece.type == Piece::Type::KING)
					{
						auto moves = generateKingInitialMoves(Pos(i, j));
						allMoves.reserve(allMoves.size() + moves.size());
						std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
					}
					else
					{
						auto moves = piece.getPossibleMoves(Pos(i, j), board);
						allMoves.reserve(allMoves.size() + moves.size());
						std::move(moves.begin(), moves.end(), std::inserter(allMoves, allMoves.end()));
					}
				}
			}

		return allMoves;
	}

	std::vector<Pos> generateKingInitialMoves(Pos pos)
	{
		std::vector<Pos> moves;
		moves.reserve(8);

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

		if (pos.x > 0 && pos.y > 0)
			moves.emplace_back(pos.x - 1, pos.y - 1);

		if (pos.x > 0 && pos.y < 7)
			moves.emplace_back(pos.x - 1, pos.y + 1);

		return moves;
	}

	void generateKingMoves(const Piece &piece, const Pos &pos, std::vector<Pos> &moves, const Board &board)
	{
		auto initialMoves = generateKingInitialMoves(pos);

		{
			const auto iterator = std::remove_if(initialMoves.begin(), initialMoves.end(),
				[&piece, &board](const Pos &pos) { return board[pos] && piece.hasSameColor(board[pos]); });
			initialMoves.erase(iterator, initialMoves.end());
		}

		if (initialMoves.empty()) return;
		std::move(initialMoves.begin(), initialMoves.end(), std::back_inserter(moves));

		// Remove the possible moves of the opponent's kings
		auto opponentsMoves = generateKingInitialMoves(Player::getKingPos(!piece.isWhite, board));

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

				if (std::find(opponentsMoves.begin(), opponentsMoves.end(), posCopy) != opponentsMoves.end())
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
	}

}
