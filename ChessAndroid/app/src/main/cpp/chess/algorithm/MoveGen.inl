#pragma once

#include "MoveGen.h"

#include "PieceAttacks.h"
#include "../data/Board.h"
#include "../data/Bitboard.h"

template <GenType T>
U64 MoveGen<T>::getPawnMoves(const Color color, const byte square, const Board &board) noexcept
{
	using namespace Bitboard;
	U64 attacks = PieceAttacks::getPawnAttacks(color, square);

	if constexpr (T == ALL || T == CAPTURES)
	{
		U64 captures = attacks & board.allPieces[~color];

		if (board.enPassantSq < 64u)
		{
			const U64 enPassant = shiftedBoards[board.enPassantSq];
			const U64 capturedPawn = color ? shift<SOUTH>(enPassant) : shift<NORTH>(enPassant);

			if (board.getType(~color, PAWN) & capturedPawn)
			{
				// Keep the en-passant capture if it intersect with one of our potential attacks
				captures |= attacks & enPassant;
			}
		}
		
		attacks = captures;
	}
	else if constexpr (T == ATTACKS_DEFENSES)
		attacks &= board.occupied;

	if constexpr (T == ALL)
	{
		const U64 bitboard = Bitboard::shiftedBoards[square];
		const U64 move = color ? shift<NORTH>(bitboard) : shift<SOUTH>(bitboard);

		if (!(board.occupied & move))
		{
			attacks |= move;

			const U64 initialRank = color ? RANK_2 : RANK_7;
			if (initialRank & bitboard)
			{
				const U64 doubleMove = color ? shift<NORTH>(move) : shift<SOUTH>(move);

				if (!(board.occupied & doubleMove))
					attacks |= doubleMove;
			}
		}
	}
	
	return attacks;
}

template <GenType T>
U64 MoveGen<T>::getKnightMoves(const Color color, const byte square, const Board &board) noexcept
{
	U64 attacks = PieceAttacks::getKnightAttacks(square);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[color]; // Remove our pieces
	else if constexpr (T == CAPTURES)
		attacks &= board.allPieces[~color]; // Keep only their pieces
	else if constexpr (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::getBishopMoves(const Color color, const byte square, const Board &board) noexcept
{
	U64 attacks = PieceAttacks::getBishopAttacks(square, board.occupied);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[color]; // Remove our pieces
	else if constexpr (T == CAPTURES)
		attacks &= board.allPieces[~color]; // Keep only their pieces
	else if constexpr (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::getRookMoves(const Color color, const byte square, const Board &board) noexcept
{
	U64 attacks = PieceAttacks::getRookAttacks(square, board.occupied);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[color]; // Remove our pieces
	else if constexpr (T == CAPTURES)
		attacks &= board.allPieces[~color]; // Keep only their pieces
	else if constexpr (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::getQueenMoves(const Color color, const byte square, const Board &board) noexcept
{
	U64 attacks = PieceAttacks::getBishopAttacks(square, board.occupied)
				| PieceAttacks::getRookAttacks(square, board.occupied);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[color]; // Remove our pieces
	else if constexpr (T == CAPTURES)
		attacks &= board.allPieces[~color]; // Keep only their pieces
	else if constexpr (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::getKingMoves(const Color color, const byte square, const Board &board) noexcept
{
	U64 attacks = PieceAttacks::getKingAttacks(square);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[color]; // Remove our pieces
	else if constexpr (T == CAPTURES)
		attacks &= board.allPieces[~color]; // Keep only their pieces
	else if constexpr (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	if constexpr (T == CAPTURES || T == ATTACKS_DEFENSES)
		return attacks;

	U64 opponentsMoves{};
	{
		const Color opponentColor = ~color;
		U64 attacksCopy = attacks;
		
		while (attacksCopy)
		{
			const byte currentSquare = Bitboard::findNextSquare(attacksCopy);
			if (Player::isAttacked(opponentColor, currentSquare, board))
				opponentsMoves |= Bitboard::shiftedBoards[currentSquare];
		}

		attacks &= ~opponentsMoves;
	}

	if (!attacks)
		return 0ull;

	// Castling
	const U64 bitboard = Bitboard::shiftedBoards[square];
	const State checkState = color ? State::WHITE_IN_CHECK : State::BLACK_IN_CHECK;

	if (board.state == checkState || !board.canCastle(color) || opponentsMoves & bitboard)
		return attacks;
	
	const byte y = row(square);
	const auto isEmptyAndCheckFree = [&, y](const byte x) {
		const byte sq = toSquare(x, y);
		const U64 bb = Bitboard::shiftedBoards[sq];
		 
		return !board.getPiece(x, y) && !(opponentsMoves & bb)
			&& !Player::isAttacked(~color, sq, board);
	};

	// King Side
	if (board.canCastleKs(color)
		&& isEmptyAndCheckFree(5)
		&& isEmptyAndCheckFree(6))
	{
		attacks |= Pos(6, y).toBitboard();
	}

	// Queen Side
	if (board.canCastleQs(color)
		&& isEmptyAndCheckFree(3)
		&& isEmptyAndCheckFree(2)
		&& !board.getPiece(1, y))
	{
		attacks |= Pos(2, y).toBitboard();
	}

	return attacks;
}
