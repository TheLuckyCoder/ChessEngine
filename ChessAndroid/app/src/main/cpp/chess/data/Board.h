#pragma once

#include <array>
#include <string>

#include "Piece.h"
#include "Move.h"
#include "../algorithm/PieceAttacks.h"

class UndoMove final
{
public:
	U64 zKey{};
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

	bool isRepetition() const noexcept;
	Phase getPhase() const noexcept;

	bool makeMove(Move move) noexcept;
	void undoMove() noexcept;
	void makeNullMove() noexcept;
	void undoNullMove() noexcept;

	template <PieceType>
	bool isAttacked(Color colorAttacking, byte targetSquare) const noexcept;
	bool isAttackedByAny(Color colorAttacking, byte targetSquare) const noexcept;
	bool isSideInCheck() const noexcept;
	bool isInCheck(Color color) const noexcept;
	template <Color C>
	bool isInCheck() const noexcept;

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
			   ? PieceAttacks::pawnAttacks<WHITE>(bb) : PieceAttacks::pawnAttacks<BLACK>(bb);
	} else if constexpr (P == KNIGHT)
		return type & PieceAttacks::knightAttacks(targetSquare);
	else if constexpr (P == BISHOP)
		return type & PieceAttacks::bishopAttacks(targetSquare, occupied);
	else if constexpr (P == ROOK)
		return type & PieceAttacks::rookAttacks(targetSquare, occupied);
	else if constexpr (P == QUEEN)
		return type & PieceAttacks::queenAttacks(targetSquare, occupied);
	else if constexpr (P == KING)
		return type & PieceAttacks::kingAttacks(targetSquare);

	assert(false);
	return false;
}
