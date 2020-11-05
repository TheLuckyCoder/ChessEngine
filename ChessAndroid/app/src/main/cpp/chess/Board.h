#pragma once

#include <array>
#include <string>

#include "Piece.h"
#include "Move.h"
#include "algorithm/Attacks.h"

class UndoMove final
{
public:
	u64 zKey{};
	Bitboard kingAttackers{};
	u32 moveContents{};
	u8 castlingRights{};
	Square enPassantSq{};
	u8 fiftyMoveRule{};

	Move getMove() const noexcept
	{
		return Move(moveContents);
	}
};

class Board final
{
public:
	u64 zKey{};
	Bitboard occupied{};
	Bitboard kingAttackers{};
	std::array<Bitboard, 2> allPieces{};
	std::array<std::array<Bitboard, 7>, 2> pieces{};

	std::array<std::array<u8, 16>, 15> pieceList{};
	std::array<u8, 15> pieceCount{};

	std::array<Piece, 64> data{};

	short npm{};
	Color colorToMove{};
	u8 castlingRights{};
	Square enPassantSq{};
	u8 fiftyMoveRule{};

	short ply{};
	short historyPly{};
	UndoMove history[MAX_MOVES]{};

	void setToStartPos();
	bool setToFen(const std::string &fen);
	std::string getFen() const;

	bool canCastle(Color color) const noexcept;
	template<Color C>
	bool canCastle() const noexcept;
	template<Color C>
	bool canCastleKs() const noexcept;
	template<Color C>
	bool canCastleQs() const noexcept;
	template<Color C>
	bool isCastled() const noexcept;

	Piece &getPiece(Square square) noexcept;
	Piece getPiece(Square square) const noexcept;
	Bitboard &getType(Piece piece) noexcept;
	Bitboard &getType(PieceType type, Color color) noexcept;
	Bitboard getType(PieceType type, Color color) const noexcept;
	template<Color C>
	Square getKingSq() const noexcept;

	bool isDrawn() const noexcept;
	Phase getPhase() const noexcept;

	bool makeMove(Move move) noexcept;
	void undoMove() noexcept;
	void makeNullMove() noexcept;
	void undoNullMove() noexcept;

	template<PieceType>
	bool isAttacked(Color attackerColor, Square targetSquare) const noexcept;
	bool isAttackedByAny(Color attackerColor, Square targetSquare) const noexcept;
	template<Color C>
	Bitboard allKingAttackers() const noexcept;
	bool isSideInCheck() const noexcept;

private:
	void addPiece(Square square, Piece piece) noexcept;
	void movePiece(Square from, Square to) noexcept;
	void removePiece(Square square) noexcept;

public:
	void updatePieceList() noexcept;
	void updateNonPieceBitboards() noexcept;

	std::string printBoard() const noexcept;
};

template<Color C>
bool Board::canCastle() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLE_BLACK_BOTH : CASTLE_WHITE_BOTH);
}

template<Color C>
bool Board::canCastleKs() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLE_BLACK_KING : CASTLE_WHITE_KING);
}

template<Color C>
bool Board::canCastleQs() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLE_BLACK_QUEEN : CASTLE_WHITE_QUEEN);
}

template<Color C>
bool Board::isCastled() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLED_BLACK : CASTLED_WHITE);
}

template<Color C>
Square Board::getKingSq() const noexcept
{
	return toSquare(pieceList[Piece{ KING, C }][0]);
}

template<PieceType P>
bool Board::isAttacked(const Color attackerColor, const Square targetSquare) const noexcept
{
	static_assert(PAWN <= P);
	static_assert(P <= KING);

	const auto type = getType(P, attackerColor);

	if constexpr (P == PAWN)
	{
		const auto pawnAttacks = attackerColor
								 ? Attacks::pawnAttacks<WHITE>(type) : Attacks::pawnAttacks<BLACK>(type);

		return bool(pawnAttacks & Bitboard::fromSquare(targetSquare));
	} else if constexpr (P == KNIGHT)
		return bool(type & Attacks::knightAttacks(targetSquare));
	else if constexpr (P == BISHOP)
		return bool(type & Attacks::bishopAttacks(targetSquare, occupied));
	else if constexpr (P == ROOK)
		return bool(type & Attacks::rookAttacks(targetSquare, occupied));
	else if constexpr (P == QUEEN)
		return bool(type & Attacks::queenAttacks(targetSquare, occupied));
	else if constexpr (P == KING)
		return bool(type & Attacks::kingAttacks(targetSquare));

	return false;
}

template<Color C>
Bitboard Board::allKingAttackers() const noexcept
{
	constexpr Color ColorAttacking = ~C;
	const Square kingSq = getKingSq<C>();
	assert(kingSq < SQUARE_NB);

	const auto queens = getType(QUEEN, ColorAttacking);
	const auto bishops = getType(BISHOP, ColorAttacking) | queens;
	const auto rooks = getType(ROOK, ColorAttacking) | queens;

	return (getType(PAWN, ColorAttacking) & Attacks::pawnAttacks<C>(Bitboard::fromSquare(kingSq)))
		   | (getType(KNIGHT, ColorAttacking) & Attacks::knightAttacks(kingSq))
		   | (getType(KING, ColorAttacking) & Attacks::kingAttacks(kingSq))
		   | (bishops & Attacks::bishopAttacks(kingSq, occupied))
		   | (rooks & Attacks::rookAttacks(kingSq, occupied));
}
