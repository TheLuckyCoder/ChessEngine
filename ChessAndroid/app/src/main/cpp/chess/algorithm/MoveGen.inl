#pragma once

#include "MoveGen.h"

#include "../data/Board.h"

template<GenType T>
PosVector<4> MoveGen<T>::generatePawnMoves(const Piece &piece, Pos pos, const Board &board)
{
	PosVector<4> moves;

	piece.isWhite ? pos.y++ : pos.y--;
	if constexpr (T == ALL)
	{
		if (!board[pos])
		{
			moves.push_back(pos);

			if (!piece.moved) {
				Pos posCopy = pos;

				piece.isWhite ? posCopy.y++ : posCopy.y--;
				if (posCopy.isValid() && !board[posCopy])
					moves.push_back(posCopy);
			}
		}
	}

	const auto handleCapture = [&piece, &moves](const Piece &other, const Pos &pos) {
		if constexpr (T == ALL || T == CAPTURES || T == KING_DANGER)
		{
			if (other && !piece.hasSameColor(other))
				moves.push_back(pos);
		}
		else if (T == ATTACKS_DEFENSES)
		{
			if (other)
				moves.push_back(pos);
		}
	};

	piece.isWhite ? pos.x-- : pos.x++;
	if (pos.isValid())
		handleCapture(board[pos], pos);

	pos.x += piece.isWhite ? 2 : -2;
	if (pos.isValid())
		handleCapture(board[pos], pos);

	return moves;
}

template<GenType T>
PosVector<8> MoveGen<T>::generateKnightMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<8> moves;

	const auto addPosIfValid = [&](const byte x, const byte y) {
		Pos newPos(pos, x, y);

		if (newPos.isValid())
		{
			const auto &other = board[newPos];

			if constexpr (T == ALL || T == KING_DANGER)
			{
				if (!other || !piece.hasSameColor(other))
					moves.push_back(newPos);
			}
			else if (T == CAPTURES)
			{
				if (other && !piece.hasSameColor(other))
					moves.push_back(newPos);
			}
			else if (T == ATTACKS_DEFENSES)
			{
				if (other)
					moves.push_back(newPos);
			}
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

template<GenType T>
PosVector<13> MoveGen<T>::generateBishopMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<13> moves;
	Pos posCopy = pos;

	const auto handleCase = [&](const Pos &newPos) {
		const auto &other = board[newPos];

		if constexpr (T == ALL)
		{
			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(newPos);
				return true;
			}
			moves.push_back(newPos);
		}
		else if (T == CAPTURES)
		{
			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(newPos);
				return true;
			}
		}
		else if (T == ATTACKS_DEFENSES)
		{
			if (other)
			{
				moves.push_back(newPos);
				return true;
			}
		}
		else if (T == KING_DANGER)
		{
			if (other)
			{
				if (!piece.hasSameColor(other))
				{
					moves.push_back(newPos);
					if (other.type != Piece::Type::KING)
						return true;
				} else
					return true;
			}
			else
				moves.push_back(newPos);
		}

		return false;
	};

	while (posCopy.x < 7 && posCopy.y > 0)
	{
		posCopy.x++;
		posCopy.y--;

		if (handleCase(posCopy))
			break;
	}

	posCopy = pos;
	while (posCopy.x < 7 && posCopy.y < 7)
	{
		posCopy.x++;
		posCopy.y++;

		if (handleCase(posCopy))
			break;
	}

	posCopy = pos;
	while (posCopy.x > 0 && posCopy.y > 0)
	{
		posCopy.x--;
		posCopy.y--;

		if (handleCase(posCopy))
			break;
	}

	posCopy = pos;
	while (posCopy.x > 0 && posCopy.y < 7)
	{
		posCopy.x--;
		posCopy.y++;

		if (handleCase(posCopy))
			break;
	}

	return moves;
}

template<GenType T>
PosVector<14> MoveGen<T>::generateRookMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<14> moves;
	Pos posCopy = pos;

	const auto handleCase = [&](const Pos &newPos) -> bool {
		const auto &other = board[newPos];

		if constexpr (T == ALL)
		{
			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(newPos);
				return true;
			}
			moves.push_back(newPos);
		}
		else if (T == CAPTURES)
		{
			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(newPos);
				return true;
			}
		}
		else if (T == ATTACKS_DEFENSES)
		{
			if (other)
			{
				moves.push_back(newPos);
				return true;
			}
		}
		else if (T == KING_DANGER)
		{
			if (other)
			{
				if (!piece.hasSameColor(other))
				{
					moves.push_back(newPos);
					if (other.type != Piece::Type::KING)
						return true;
				}
				else
					return true;
			}
			else
				moves.push_back(newPos);
		}

		return false;
	};

	while (posCopy.x > 0)
	{
		posCopy.x--;

		if (handleCase(posCopy))
			break;
	}

	posCopy = pos;
	while (posCopy.x < 7)
	{
		posCopy.x++;

		if (handleCase(posCopy))
			break;
	}

	posCopy = pos;
	while (posCopy.y > 0)
	{
		posCopy.y--;

		if (handleCase(posCopy))
			break;
	}

	posCopy = pos;
	while (posCopy.y < 7)
	{
		posCopy.y++;

		if (handleCase(posCopy))
			break;
	}

	return moves;
}

template<GenType T>
PosVector<27> MoveGen<T>::generateQueenMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<27> moves;
	moves += generateRookMoves(piece, pos, board);
	moves += generateBishopMoves(piece, pos, board);
	return moves;
}

template<GenType T>
PosVector<8> MoveGen<T>::generateKingMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<8> moves;

	const auto handleCase = [&](const byte x, const byte y) {

		const Pos newPos(x, y);
		const auto &other = board[newPos];

		if constexpr (T == ALL)
		{
			if (other)
			{
				if (!piece.hasSameColor(other))
					moves.push_back(newPos);
			} else
				moves.push_back(newPos);
		}
		else if (T == CAPTURES)
		{
			if (other && !piece.hasSameColor(other))
				moves.push_back(newPos);
		}
		else if (T == ATTACKS_DEFENSES)
		{
			if (other)
				moves.push_back(newPos);
		}
		else if (T == KING_DANGER)
			moves.emplace_back(x, y);
	};

	// Vertical and Horizontal
	if (pos.x > 0)
		handleCase(pos.x - 1u, pos.y);

	if (pos.x < 7)
		handleCase(pos.x + 1u, pos.y);

	if (pos.y > 0)
		handleCase(pos.x, pos.y - 1u);

	if (pos.y < 7)
		handleCase(pos.x, pos.y + 1u);

	// Diagonal
	if (pos.x < 7 && pos.y > 0)
		handleCase(pos.x + 1u, pos.y - 1u);

	if (pos.x < 7 && pos.y < 7)
		handleCase(pos.x + 1u, pos.y + 1u);

	if (pos.x > 0 && pos.y > 0)
		handleCase(pos.x - 1u, pos.y - 1u);

	if (pos.x > 0 && pos.y < 7)
		handleCase(pos.x - 1u, pos.y + 1u);

	if constexpr (T == CAPTURES || T == ATTACKS_DEFENSES || T == KING_DANGER) return moves;
	if (moves.empty()) return moves;

	const auto opponentsMoves = MoveGen<KING_DANGER>::getAttacksPerColorBitboard(!piece.isWhite, board);

	// Remove Attacked Positions
	for (unsigned int i = 0; i < moves.size(); i++)
	{
		if (opponentsMoves & moves[i].toBitboard())
		{
			moves[i] = moves.back();
			moves.pop_back();
			i--;
		}
	}

	// Castling
	if (!piece.moved && !(opponentsMoves & pos.toBitboard()))
	{
		const auto y = pos.y;
		const auto isEmptyAndChessFree = [&, y](const byte x) {
			return !board.data[x][y] && !(opponentsMoves & Pos(x, y).toBitboard());
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

template<GenType T>
Bitboard MoveGen<T>::getAttacksPerColorBitboard(const bool white, const Board &board)
{
	Bitboard bitboard{};

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
		{
			const Pos pos(x, y);
			const auto &piece = board[pos];
			if (piece && piece.isWhite == white)
			{
				Piece::MaxMovesVector moves;
				switch (piece.type)
				{
				case Piece::Type::PAWN:
					moves = MoveGen<CAPTURES>::generatePawnMoves(piece, pos, board);
					break;
				case Piece::Type::KNIGHT:
					moves = generateKnightMoves(piece, pos, board);
					break;
				case Piece::Type::BISHOP:
					moves = generateBishopMoves(piece, pos, board);
					break;
				case Piece::Type::ROOK:
					moves = generateRookMoves(piece, pos, board);
					break;
				case Piece::Type::QUEEN:
					moves = generateQueenMoves(piece, pos, board);
					break;
				case Piece::Type::KING:
					moves = MoveGen<KING_DANGER>::generateKingMoves(piece, pos, board);
					break;
				case Piece::Type::NONE:
					break;
				}

				for (const auto &move : moves)
					bitboard |= move.toBitboard();
			}
		}

	return bitboard;
}

template<GenType T>
PosMap MoveGen<T>::getAttacksPerColorMap(const bool white, const Board &board)
{
	PosMap allMoves;

	for (byte x = 0; x < 8; x++)
		for (byte y = 0; y < 8; y++)
		{
			const Pos pos(x, y);
			const auto &piece = board[pos];

			if (piece && piece.isWhite == white)
			{
				Piece::MaxMovesVector moves;
				switch (piece.type)
				{
				case Piece::Type::PAWN:
					moves = MoveGen<CAPTURES>::generatePawnMoves(piece, pos, board);
					break;
				case Piece::Type::KNIGHT:
					moves = generateKnightMoves(piece, pos, board);
					break;
				case Piece::Type::BISHOP:
					moves = generateBishopMoves(piece, pos, board);
					break;
				case Piece::Type::ROOK:
					moves = generateRookMoves(piece, pos, board);
					break;
				case Piece::Type::QUEEN:
					moves = generateQueenMoves(piece, pos, board);
					break;
				case Piece::Type::KING:
					moves = MoveGen<KING_DANGER>::generateKingMoves(piece, pos, board);
					break;
				case Piece::Type::NONE:
					break;
				}
				for (const auto &move : moves)
					allMoves[move]++;
			}
		}

	return allMoves;
}
