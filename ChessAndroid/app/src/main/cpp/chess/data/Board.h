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
	Move move{};
	byte castlingRights{};
	byte enPassantSq{};
	byte fiftyMoveRule{};
};

class Board final
{
	friend class FenParser;

public:
	std::array<Piece, 64> data{};
	U64 zKey{};
	U64 occupied{};
	std::array<U64, 2> allPieces{};
	std::array<std::array<U64, 7>, 2> pieces{};
	
	std::array<std::array<byte, 16>, 15> pieceList{};
	std::array<byte, 15> pieceCount{};
	
	short npm{};
	Color colorToMove = WHITE;
	byte castlingRights = CastlingRights::CASTLE_WHITE_BOTH | CastlingRights::CASTLE_BLACK_BOTH;
	byte fiftyMoveRule{};
	byte enPassantSq{};

	short ply{};
	short historyPly{};
	UndoMove history[MAX_MOVES]{};

	void initDefaultBoard();
	void setToFen(const std::string &fen);

	bool canCastle(Color color) const noexcept;
	bool canCastleKs(Color color) const noexcept;
	bool canCastleQs(Color color) const noexcept;
	bool isCastled(Color color) const noexcept;

	Piece &getPiece(byte squareIndex) noexcept;
	Piece getPiece(byte squareIndex) const noexcept;
	Piece &getPiece(byte x, byte y) noexcept;
	Piece getPiece(byte x, byte y) const noexcept;
	Piece at(byte x, byte y) const noexcept;
	U64 &getType(Piece piece) noexcept;
	U64 getType(Piece piece) const noexcept;
	U64 &getType(Color color, PieceType type) noexcept;
	U64 getType(Color color, PieceType type) const noexcept;
	byte getKingSq(Color color) const noexcept;

	bool isRepetition() const noexcept;
	Phase getPhase() const noexcept;

	bool makeMove(Move move, bool checkLegal = true) noexcept;
	void undoMove() noexcept;
	void makeNullMove() noexcept;
	void undoNullMove() noexcept;

	template <PieceType>
	bool isAttacked(Color colorAttacking, byte targetSquare) const noexcept;
	bool isAttackedByAny(Color colorAttacking, byte targetSquare) const noexcept;
	bool isInCheck(Color color) const noexcept;

private:
	void addPiece(byte square, Piece piece) noexcept;
	void movePiece(byte from, byte to) noexcept;
	void removePiece(byte square) noexcept;
	
	void updatePieceList() noexcept;
	void updateNonPieceBitboards() noexcept;
};

template <PieceType P>
bool Board::isAttacked(const Color colorAttacking, const byte targetSquare) const noexcept
{
	static_assert(PAWN <= P);
	static_assert(P <= KING);

	const U64 type = getType(colorAttacking, P);

	if constexpr (P == PAWN)
		return type & PieceAttacks::getPawnAttacks(~colorAttacking, targetSquare);
	else if constexpr (P == KNIGHT)
		return type & PieceAttacks::getKnightAttacks(targetSquare);
	else if constexpr (P == BISHOP)
		return type & PieceAttacks::getBishopAttacks(targetSquare, occupied);
	else if constexpr (P == ROOK)
		return type & PieceAttacks::getRookAttacks(targetSquare, occupied);
	else if constexpr (P == QUEEN)
		return type & PieceAttacks::getQueenAttacks(targetSquare, occupied);
	else if constexpr (P == KING)
		return type & PieceAttacks::getKingAttacks(targetSquare);

	assert(false);
	return false;
}
