#pragma once

#include <array>
#include <string>

#include "Piece.h"
#include "Move.h"
#include "../algorithm/Attacks.h"

class UndoMove final
{
public:
	U64 zKey{};
	U64 kingAttackers{};
	unsigned int moveContents{};
	byte castlingRights{};
	byte enPassantSq{};
	byte fiftyMoveRule{};

	Move getMove() const noexcept
	{
		return Move(moveContents);
	}
};

class Board final
{
	friend class FenParser;

public:
	U64 zKey{};
	U64 occupied{};
	U64 kingAttackers{};
	std::array<U64, 2> allPieces{};
	std::array<std::array<U64, 7>, 2> pieces{};
	
	std::array<std::array<byte, 16>, 15> pieceList{};
	std::array<byte, 15> pieceCount{};

	std::array<Piece, 64> data{};

	short npm{};
	Color colorToMove{};
	byte castlingRights{};
	byte fiftyMoveRule{};
	byte enPassantSq{};

	short ply{};
	short historyPly{};
	UndoMove history[MAX_MOVES]{};

	void initDefaultBoard();
	void setToFen(const std::string &fen);

	bool canCastle(Color color) const noexcept;
	template <Color C>
	bool canCastle() const noexcept;
	template <Color C>
	bool canCastleKs() const noexcept;
	template <Color C>
	bool canCastleQs() const noexcept;
	template <Color C>
	bool isCastled() const noexcept;

	Piece &getPiece(byte squareIndex) noexcept; 
	Piece getPiece(byte squareIndex) const noexcept;
	U64 &getType(Piece piece) noexcept;
	U64 getType(Piece piece) const noexcept;
	U64 &getType(PieceType type, Color color) noexcept;
	U64 getType(PieceType type, Color color) const noexcept;
	byte getKingSq(Color color) const noexcept;

	bool isDrawn() const noexcept;
	Phase getPhase() const noexcept;

	bool makeMove(Move move) noexcept;
	void undoMove() noexcept;
	void makeNullMove() noexcept;
	void undoNullMove() noexcept;

	template <PieceType>
	bool isAttacked(Color colorAttacking, byte targetSquare) const noexcept;
	bool isAttackedByAny(Color attackerColor, byte targetSquare) const noexcept;
	template <Color C>
	U64 allKingAttackers() const noexcept;
	bool isSideInCheck() const noexcept;

private:
	void addPiece(byte square, Piece piece) noexcept;
	void movePiece(byte from, byte to) noexcept;
	void removePiece(byte square) noexcept;
	
	void updatePieceList() noexcept;
	void updateNonPieceBitboards() noexcept;
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

template <PieceType P>
bool Board::isAttacked(const Color colorAttacking, const byte targetSquare) const noexcept
{
	static_assert(PAWN <= P);
	static_assert(P <= KING);

	const U64 type = getType(P, colorAttacking);

	if constexpr (P == PAWN)
	{
		const U64 bb = Bits::getSquare64(targetSquare);
		return type & ~colorAttacking
			   ? Attacks::pawnAttacks<WHITE>(bb) : Attacks::pawnAttacks<BLACK>(bb);
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
U64 Board::allKingAttackers() const noexcept
{
	constexpr Color ColorAttacking = ~C;
	const byte kingSq = getKingSq(C);
	assert(kingSq < SQUARE_NB);

	const U64 queens = getType(QUEEN, ColorAttacking);
	const U64 bishops = getType(BISHOP, ColorAttacking) | queens;
	const U64 rooks = getType(ROOK, ColorAttacking) | queens;

	return (getType(PAWN, ColorAttacking) & Attacks::pawnAttacks<C>(Bits::getSquare64(kingSq)))
		   | (getType(KNIGHT, ColorAttacking) & Attacks::knightAttacks(kingSq))
		   | (getType(KING, ColorAttacking) & Attacks::kingAttacks(kingSq))
		   | (bishops & Attacks::bishopAttacks(kingSq, occupied))
		   | (rooks & Attacks::rookAttacks(kingSq, occupied));
}
