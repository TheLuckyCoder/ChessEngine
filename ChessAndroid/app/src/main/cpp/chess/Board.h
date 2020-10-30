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
	u64 kingAttackers{};
	unsigned int moveContents{};
	u8 castlingRights{};
	u8 enPassantSq{};
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
	u64 occupied{};
	u64 kingAttackers{};
	std::array<u64, 2> allPieces{};
	std::array<std::array<u64, 7>, 2> pieces{};
	
	std::array<std::array<u8, 16>, 15> pieceList{};
	std::array<u8, 15> pieceCount{};

	std::array<Piece, 64> data{};

	short npm{};
	Color colorToMove{};
	u8 castlingRights{};
	u8 fiftyMoveRule{};
	u8 enPassantSq{};

	short ply{};
	short historyPly{};
	UndoMove history[MAX_MOVES]{};

	void setToStartPos();
	bool setToFen(const std::string &fen);
	std::string getFen() const;

	bool canCastle(Color color) const noexcept;
	template <Color C>
	bool canCastle() const noexcept;
	template <Color C>
	bool canCastleKs() const noexcept;
	template <Color C>
	bool canCastleQs() const noexcept;
	template <Color C>
	bool isCastled() const noexcept;

	Piece &getPiece(u8 squareIndex) noexcept;
	Piece getPiece(u8 squareIndex) const noexcept;
	u64 &getType(Piece piece) noexcept;
	u64 &getType(PieceType type, Color color) noexcept;
	u64 getType(PieceType type, Color color) const noexcept;
	template <Color C>
	u8 getKingSq() const noexcept;

	bool isDrawn() const noexcept;
	Phase getPhase() const noexcept;

	bool makeMove(Move move) noexcept;
	void undoMove() noexcept;
	void makeNullMove() noexcept;
	void undoNullMove() noexcept;

	template <PieceType>
	bool isAttacked(Color attackerColor, u8 targetSquare) const noexcept;
	bool isAttackedByAny(Color attackerColor, u8 targetSquare) const noexcept;
	template <Color C>
	u64 allKingAttackers() const noexcept;
	bool isSideInCheck() const noexcept;

private:
	void addPiece(u8 square, Piece piece) noexcept;
	void movePiece(u8 from, u8 to) noexcept;
	void removePiece(u8 square) noexcept;

public:
	void updatePieceList() noexcept;
	void updateNonPieceBitboards() noexcept;

	std::string printBoard() const noexcept;
};

template <Color C>
bool Board::canCastle() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLE_BLACK_BOTH : CASTLE_WHITE_BOTH);
}

template <Color C>
bool Board::canCastleKs() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLE_BLACK_KING : CASTLE_WHITE_KING);
}

template <Color C>
bool Board::canCastleQs() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLE_BLACK_QUEEN : CASTLE_WHITE_QUEEN);
}

template <Color C>
bool Board::isCastled() const noexcept
{
	return castlingRights & (C == BLACK ? CASTLED_BLACK : CASTLED_WHITE);
}

template <Color C>
u8 Board::getKingSq() const noexcept
{
	return pieceList[Piece{ KING, C }][0];
}

template <PieceType P>
bool Board::isAttacked(const Color attackerColor, const u8 targetSquare) const noexcept
{
	static_assert(PAWN <= P);
	static_assert(P <= KING);

	const u64 type = getType(P, attackerColor);

	if constexpr (P == PAWN)
	{
		const u64 bb = Bits::getSquare64(targetSquare);
		const u64 pawnAttacks = attackerColor
			   ? Attacks::pawnAttacks<WHITE>(type) : Attacks::pawnAttacks<BLACK>(type);

		return pawnAttacks & bb;
	} else if constexpr (P == KNIGHT)
		return type & Attacks::knightAttacks(targetSquare);
	else if constexpr (P == BISHOP)
		return type & Attacks::bishopAttacks(targetSquare, occupied);
	else if constexpr (P == ROOK)
		return type & Attacks::rookAttacks(targetSquare, occupied);
	else if constexpr (P == QUEEN)
		return type & Attacks::queenAttacks(targetSquare, occupied);
	else if constexpr (P == KING)
		return type & Attacks::kingAttacks(targetSquare);

	return false;
}

template <Color C>
u64 Board::allKingAttackers() const noexcept
{
	constexpr Color ColorAttacking = ~C;
	const u8 kingSq = getKingSq<C>();
	assert(kingSq < SQUARE_NB);

	const u64 queens = getType(QUEEN, ColorAttacking);
	const u64 bishops = getType(BISHOP, ColorAttacking) | queens;
	const u64 rooks = getType(ROOK, ColorAttacking) | queens;

	return (getType(PAWN, ColorAttacking) & Attacks::pawnAttacks<C>(Bits::getSquare64(kingSq)))
		   | (getType(KNIGHT, ColorAttacking) & Attacks::knightAttacks(kingSq))
		   | (getType(KING, ColorAttacking) & Attacks::kingAttacks(kingSq))
		   | (bishops & Attacks::bishopAttacks(kingSq, occupied))
		   | (rooks & Attacks::rookAttacks(kingSq, occupied));
}
