#pragma once

#include "MoveGen.h"

#include "PieceAttacks.h"
#include "../data/Board.h"
#include "../data/Bitboard.h"

template <GenType T>
U64 MoveGen<T>::generatePawnMoves(const Piece &piece, byte square, const Board &board)
{
	U64 attacks = PieceAttacks::getPawnAttacks(piece.isWhite, square);

	if constexpr (T == ALL || T == CAPTURES)
	{
		U64 captures = attacks & board.allPieces[!piece.isWhite];
		
		// Keep the en-passant capture if it intersect with one of our potential attacks
		captures |= attacks & board.enPassant;
		attacks = captures;
	}
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied;

	const U64 initialBb = Bitboard::shiftedBoards[square];
	Pos pos(square);
	piece.isWhite ? pos.y++ : pos.y--;

	if constexpr (T == ALL)
	{
		if (!board[pos])
		{
			attacks |= pos.toBitboard();

			const U64 initialRank = piece.isWhite ? RANK_2 : RANK_7;
			if (initialRank & initialBb)
			{
				byte y = pos.y;

				piece.isWhite ? y++ : y--;
				if (y < 8u && !board.getPiece(pos.x, y))
					attacks |= Pos(pos.x, y).toBitboard();
			}
		}
	}
	
	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateKnightMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 attacks = PieceAttacks::getKnightAttacks(square);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateBishopMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 attacks = PieceAttacks::getBishopAttacks(square, board.occupied);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateRookMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 attacks = PieceAttacks::getRookAttacks(square, board.occupied);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateQueenMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 attacks = PieceAttacks::getBishopAttacks(square, board.occupied)
				| PieceAttacks::getRookAttacks(square, board.occupied);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateKingMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 attacks = PieceAttacks::getKingAttacks(square);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	if constexpr (T == CAPTURES || T == ATTACKS_DEFENSES)
		return attacks;

	U64 opponentsMoves{};
	{
		const Color opponentColor = oppositeColor(toColor(piece.isWhite));
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
	const Color color = toColor(piece.isWhite);
	const State checkState = piece.isWhite ? State::WHITE_IN_CHECK : State::BLACK_IN_CHECK;

	if (board.state == checkState || !board.canCastle(color) || opponentsMoves & bitboard)
		return attacks;
	
	const byte y = row(square);
	const auto isEmptyAndCheckFree = [&, y](const byte x) {
		const byte sq = Pos(x, y).toSquare();
		const U64 bb = Bitboard::shiftedBoards[sq];
		 
		return !board.getPiece(x, y) && !(opponentsMoves & bb) && !Player::isAttacked(oppositeColor(color), sq, board);
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
