#include "MoveGen.h"

#include <algorithm>
#include <iterator>

#include "../Board.h"
#include "../../BoardManager.h"
#include "../../memory/Containers.h"

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

			if (!piece.moved) {
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
				const auto &other = board[startPos];

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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
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

			if (const auto &other = board[posCopy]; other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(posCopy);
				break;
			}
			moves.push_back(posCopy);
		}

		return moves;
	}

	std::unordered_set<Pos> getAllAttacksPerColor(const bool white, const Board &board)
	{
		std::unordered_set<Pos> attacks(64);

		for (byte x = 0; x < 8; x++)
			for (byte y = 0; y < 8; y++)
			{
				const Pos pos(x, y);
				const auto &piece = board[pos];
				if (piece && piece.isWhite == white)
				{
					if (piece.type == Piece::Type::PAWN)
					{
						auto moves = generatePawnAttacks(piece, pos);
						std::move(moves.begin(), moves.end(), std::inserter(attacks, attacks.end()));
					}
					else if (piece.type == Piece::Type::KING)
					{
						auto moves = generateKingInitialMoves(pos);
						std::move(moves.begin(), moves.end(), std::inserter(attacks, attacks.end()));
					}
					else
					{
						auto moves = piece.getPossibleMoves(pos, board);
						std::move(moves.begin(), moves.end(), std::inserter(attacks, attacks.end()));
					}
				}
			}

		return attacks;
	}

	std::unordered_map<Pos, short> getAllMovesPerColor(const bool white, const Board &board)
	{
		std::unordered_map<Pos, short> allMoves(64);

		for (byte x = 0; x < 8; x++)
			for (byte y = 0; y < 8; y++)
			{
				const Pos pos(x, y);
				const auto &piece = board[pos];

				if (piece && piece.isWhite == white)
				{
					if (piece.type == Piece::Type::PAWN)
					{
						const auto moves = generatePawnAttacks(piece, pos);
						for (const auto &move : moves)
							allMoves[move]++;
					}
					else if (piece.type == Piece::Type::KING)
					{
						const auto moves = generateKingInitialMoves(pos);
						for (const auto &move : moves)
							allMoves[move]++;
					}
					else
					{
						const auto moves = piece.getPossibleMoves(pos, board);
						for (const auto &move : moves)
							allMoves[move]++;
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

		const auto opponentsMoves = getAllAttacksPerColor(!piece.isWhite, board);

		for (unsigned int i = 0; i < moves.size(); i++)
		{
			if (opponentsMoves.find(moves[i]) != opponentsMoves.end())
			{
				moves[i] = moves.back();
				moves.pop_back();
				i--;
			}
		}

		// Castling
		if (!piece.moved && opponentsMoves.find(pos) == opponentsMoves.end())
		{
			const auto y = pos.y;
			const auto isEmptyAndChessFree = [&, y] (const byte x) {
				return !board.data[x][y] && opponentsMoves.find(Pos(x, y)) == opponentsMoves.end();
			};

			if (isEmptyAndChessFree(5) && isEmptyAndChessFree(6))
				if (const auto &other = board.data[7][y];
					other.type == Piece::Type::ROOK && piece.hasSameColor(other) && !other.moved)
					moves.emplace_back(6, pos.y);

			if (isEmptyAndChessFree(3) && isEmptyAndChessFree(2) && !board.data[1][y])
				if (const auto &other = board.data[0][y];
					other.type == Piece::Type::ROOK && piece.hasSameColor(other) && !other.moved)
					moves.emplace_back(2, pos.y);
		}

		return moves;
	}

}
