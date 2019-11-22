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
	else if (T == KING_DANGER) {
		// Do nothing
	} else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateBishopMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 occupancy = board.occupied;

	if constexpr (T == KING_DANGER)
		occupancy &= ~board.getType(toColor(piece.isWhite), PieceType::KING); // Remove the king

	U64 attacks = PieceAttacks::getBishopAttacks(square, occupancy);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= occupancy; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateRookMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 occupancy = board.occupied;

	if constexpr (T == KING_DANGER)
		occupancy &= ~board.getType(toColor(piece.isWhite), PieceType::KING); // Remove their king

	U64 attacks = PieceAttacks::getRookAttacks(square, occupancy);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= occupancy; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateQueenMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 occupancy = board.occupied;

	if constexpr (T == KING_DANGER)
		occupancy &= ~board.getType(toColor(piece.isWhite), PieceType::KING); // Remove their king

	U64 attacks = PieceAttacks::getBishopAttacks(square, occupancy)
				| PieceAttacks::getRookAttacks(square, occupancy);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= occupancy; // Keep only the pieces

	return attacks;
}

template <GenType T>
U64 MoveGen<T>::generateKingMoves(const Piece &piece, const byte square, const Board &board)
{
	U64 attacks = PieceAttacks::getKingAttacks(square);

	if constexpr (T == ALL)
		attacks &= ~board.allPieces[piece.isWhite]; // Remove our pieces
	else if (T == KING_DANGER) {
		// Do Nothing
	} else if (T == CAPTURES)
		attacks &= board.allPieces[!piece.isWhite]; // Keep only their pieces
	else if (T == ATTACKS_DEFENSES)
		attacks &= board.occupied; // Keep only the pieces

	if constexpr (T == CAPTURES || T == ATTACKS_DEFENSES || T == KING_DANGER)
		return attacks;

	if (attacks == 0ull)
		return attacks;

	U64 opponentsMoves{};
	MoveGen<KING_DANGER>::forEachAttack(!piece.isWhite, board, [&] (const U64 enemyAttacks) -> bool {
		opponentsMoves |= enemyAttacks;
		attacks &= ~enemyAttacks; // Remove Attacked Positions
		return static_cast<bool>(attacks);
	});

	if (!attacks) return 0ull;

	// Castling
	const Color color = toColor(piece.isWhite);
	const State checkState = piece.isWhite ? State::WHITE_IN_CHECK : State::BLACK_IN_CHECK;
	if (board.state != checkState
		&& board.canCastleAnywhere(color)
		&& !(opponentsMoves & Bitboard::shiftedBoards[square]))
	{
		const byte y = row(square);
		const auto isEmptyAndCheckFree = [&, y](const byte x) {
			return !board.getPiece(x, y) && !(opponentsMoves & Pos(x, y).toBitboard());
		};

		if (board.canCastleKs(color)
			&& isEmptyAndCheckFree(5)
			&& isEmptyAndCheckFree(6))
		{
			if (const auto &other = board.getPiece(7, y);
				other.type == PieceType::ROOK
				&& piece.isSameColor(other))
				attacks |= Pos(6, y).toBitboard();
		}

		if (board.canCastleQs(color)
			&& isEmptyAndCheckFree(3)
			&& isEmptyAndCheckFree(2)
			&& !board.getPiece(1, y))
		{
			if (const auto &other = board.getPiece(0, y);
				other.type == PieceType::ROOK
				&& piece.isSameColor(other))
				attacks |= Pos(2, y).toBitboard();
		}
	}

	return attacks;
}

template <GenType T> // Class Template
void MoveGen<T>::forEachAttack(const bool white, const Board &board, const std::function<bool(U64 attacks)> &func)
{
	for (byte i = 0; i < 64u; ++i)
	{
		const auto &piece = board.getPiece(i);
		if (piece && piece.isWhite == white)
		{
			U64 moves{};
			switch (piece.type)
			{
			case PieceType::PAWN:
				moves = MoveGen<KING_DANGER>::generatePawnMoves(piece, i, board);
				break;
			case PieceType::KNIGHT:
				moves = generateKnightMoves(piece, i, board);
				break;
			case PieceType::BISHOP:
				moves = generateBishopMoves(piece, i, board);
				break;
			case PieceType::ROOK:
				moves = generateRookMoves(piece, i, board);
				break;
			case PieceType::QUEEN:
				moves = generateQueenMoves(piece, i, board);
				break;
			case PieceType::KING:
				moves = MoveGen<KING_DANGER>::generateKingMoves(piece, i, board);
				break;
			case PieceType::NONE:
				break;
			}

			// The function should return true in order to stop the loop
			if (func(moves))
				return;
		}
	}
}

template<GenType T>
AttacksMap MoveGen<T>::getAttacksPerColor(const bool white, const Board &board)
{
	AttacksMap attacks{};

	for (byte startSq = 0; startSq < 64u; ++startSq)
	{
		const Piece &piece = board.getPiece(startSq);
		if (piece && piece.isWhite == white)
		{
			U64 moves{};
			switch (piece.type)
			{
				case PieceType::PAWN:
					moves = MoveGen<KING_DANGER>::generatePawnMoves(piece, startSq, board);
					break;
				case PieceType::KNIGHT:
					moves = generateKnightMoves(piece, startSq, board);
					break;
				case PieceType::BISHOP:
					moves = generateBishopMoves(piece, startSq, board);
					break;
				case PieceType::ROOK:
					moves = generateRookMoves(piece, startSq, board);
					break;
				case PieceType::QUEEN:
					moves = generateQueenMoves(piece, startSq, board);
					break;
				case PieceType::KING:
					moves = MoveGen<KING_DANGER>::generateKingMoves(piece, startSq, board);
					break;
				case PieceType::NONE:
					break;
			}

			while (moves)
			{
				const byte destSq = Bitboard::findNextSquare(moves);
				attacks.map[destSq]++;
				attacks.board[piece.isWhite][piece.type - 1u] |= Bitboard::shiftedBoards[destSq];
			}
		}
	}

	return attacks;
}
