#pragma once

#include "MoveGen.h"

#include "PieceAttacks.h"
#include "../data/Board.h"

template <std::size_t N>
static void bitboardAttacksToMoves(PosVector<N> &moves, U64 attacks)
{
	for (byte i = 0; attacks && i < 64; i++)
	{
		if (attacks & 1ull)
			moves.emplace_back(i / 8, i % 8);
		attacks >>= 1;
	}
}

template <GenType T, bool ToList>
auto MoveGen<T, ToList>::generatePawnMoves(const Piece &piece, Pos pos, const Board &board)
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

	const auto handleCapture = [&] {
		const auto &other = board[pos];
		if constexpr (T == ALL || T == CAPTURES || T == KING_DANGER)
		{
			if (other && !piece.isSameColor(other))
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
		handleCapture();

	pos.x += piece.isWhite ? 2 : -2;
	if (pos.isValid())
		handleCapture();

	return moves;
}

template <GenType T, bool ToList>
auto MoveGen<T, ToList>::generateKnightMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<8> moves;

	U64 attacks = PieceAttacks::getKnightAttacks(pos.toSquare());

	if constexpr (T == ALL | T == KING_DANGER)
		attacks &= ~board.pieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.pieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= (board.pieces[0] | board.pieces[1]); // Keep only the pieces

	if constexpr (ToList)
	{
		bitboardAttacksToMoves(moves, attacks);
		return moves;
	}
	else
		return attacks;
}

template <GenType T, bool ToList>
auto MoveGen<T, ToList>::generateBishopMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<13> moves;
	Pos posCopy = pos;

	const auto handleCase = [&](const Pos &newPos) {
		const auto &other = board[newPos];

		if constexpr (T == ALL)
		{
			if (other)
			{
				if (!piece.isSameColor(other))
					moves.push_back(newPos);
				return true;
			}
			moves.push_back(newPos);
		}
		else if (T == CAPTURES)
		{
			if (other)
			{
				if (!piece.isSameColor(other))
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
				if (!piece.isSameColor(other))
				{
					moves.push_back(newPos);
					if (other.type != Type::KING)
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

template <GenType T, bool ToList>
auto MoveGen<T, ToList>::generateRookMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<14> moves;
	Pos posCopy = pos;

	const auto handleCase = [&](const Pos &newPos) -> bool {
		const auto &other = board[newPos];

		if constexpr (T == ALL)
		{
			if (other)
			{
				if (!piece.isSameColor(other))
					moves.push_back(newPos);
				return true;
			}
			moves.push_back(newPos);
		}
		else if (T == CAPTURES)
		{
			if (other)
			{
				if (!piece.isSameColor(other))
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
				if (!piece.isSameColor(other))
				{
					moves.push_back(newPos);
					if (other.type != Type::KING)
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

template <GenType T, bool ToList>
auto MoveGen<T, ToList>::generateQueenMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<27> moves;
	moves += generateRookMoves(piece, pos, board);
	moves += generateBishopMoves(piece, pos, board);
	return moves;
}

template <GenType T, bool ToList>
auto MoveGen<T, ToList>::generateKingMoves(const Piece &piece, const Pos &pos, const Board &board)
{
	PosVector<10> moves;

	U64 attacks = PieceAttacks::getKingAttacks(pos.toSquare());

	if constexpr (T == ALL)
		attacks &= ~board.pieces[piece.isWhite]; // Remove our pieces
	else if (T == KING_DANGER) {
		// Do Nothing
	} else if (T == CAPTURES)
		attacks &= board.pieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= (board.pieces[0] | board.pieces[1]); // Keep only the pieces

	if constexpr (T == CAPTURES || T == ATTACKS_DEFENSES || T == KING_DANGER)
	{
		if constexpr (ToList)
		{
			bitboardAttacksToMoves(moves, attacks);
			return moves;
		}
		else
			return attacks;
	}
	if (attacks == 0ull)
	{
		if constexpr (ToList)
			return moves;
		else
			return 0ull;
	}

	U64 opponentsMoves{};
	MoveGen<KING_DANGER, ToList>::forEachAttack(!piece.isWhite, board, [&] (const Piece &/*piece*/, const Pos &move) -> bool {
		opponentsMoves |= move.toBitboard();
		return false;
	});

	attacks &= ~opponentsMoves; // Remove Attacked Positions

	// Castling
	if (!piece.moved && !(opponentsMoves & pos.toBitboard()))
	{
		const auto y = pos.y;
		const auto isEmptyAndCheckFree = [&, y](const byte x) {
			return !board.data[x][y] && !(opponentsMoves & Pos(x, y).toBitboard());
		};

		if (isEmptyAndCheckFree(5) && isEmptyAndCheckFree(6))
			if (const auto &other = board.data[7][y];
				other.type == Type::ROOK && piece.isSameColor(other) && !other.moved)
				attacks |= Pos(6, pos.y).toBitboard();

		if (isEmptyAndCheckFree(3) && isEmptyAndCheckFree(2) && !board.data[1][y])
			if (const auto &other = board.data[0][y];
				other.type == Type::ROOK && piece.isSameColor(other) && !other.moved)
				attacks |= Pos(2, pos.y).toBitboard();
	}

	if constexpr (ToList)
	{
		bitboardAttacksToMoves(moves, attacks);
		return moves;
	}
	else
		return attacks;
}

template <GenType T, bool ToList> // Class Template
template <class Func> // Function Template
void MoveGen<T, ToList>::forEachAttack(const bool white, const Board &board, Func &&func)
{
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
				case Type::PAWN:
					moves = MoveGen<KING_DANGER, ToList>::generatePawnMoves(piece, pos, board);
					break;
				case Type::KNIGHT:
					moves = generateKnightMoves(piece, pos, board);
					break;
				case Type::BISHOP:
					moves = generateBishopMoves(piece, pos, board);
					break;
				case Type::ROOK:
					moves = generateRookMoves(piece, pos, board);
					break;
				case Type::QUEEN:
					moves = generateQueenMoves(piece, pos, board);
					break;
				case Type::KING:
					moves = MoveGen<KING_DANGER, ToList>::generateKingMoves(piece, pos, board);
					break;
				case Type::NONE:
					break;
				}

				// The function should return true in order to stop the loop
				for (const auto &move : moves)
					if (func(piece, move))
						return;
			}
		}
}

template<GenType T, bool ToList>
Attacks MoveGen<T, ToList>::getAttacksPerColor(const bool white, const Board &board)
{
	Attacks attacks{};

	forEachAttack(white, board, [&] (const Piece &piece, const Pos &move) -> bool {
		const byte square = move.toSquare();

		attacks.map[square]++;
		attacks.board[piece.isWhite][piece.type - 1u] |= (1ull << square);
		return false;
	});

	return attacks;
}
