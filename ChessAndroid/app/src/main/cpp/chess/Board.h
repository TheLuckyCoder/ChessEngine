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

	std::array<Bitboard, COLOR_NB> blockersForKing{};
	std::array<Bitboard, COLOR_NB> pinners{};
	std::array<Bitboard, PIECE_TYPE_NB> checkSquares{};

	Move getMove() const noexcept { return Move{ moveContents }; }
};

class Board final
{
public:
	void setToStartPos();
	bool setToFen(const std::string &fen);
	std::string getFen() const;

	// region Castling

	bool canCastle(Color color) const noexcept;
	template <Color C>
	bool canCastle() const noexcept;
	template <Color C>
	bool canCastleKs() const noexcept;
	template <Color C>
	bool canCastleQs() const noexcept;
	template <Color C>
	bool isCastled() const noexcept;

	// endregion Castling

	// region Pieces
private:
	Piece &getPiece(Square square) noexcept;
    Bitboard &getPieces(Piece piece) noexcept;

public:
    Piece getPiece(Square square) const noexcept;
    Bitboard getPieces() const noexcept;
	Bitboard getPieces(PieceType type, Color color) const noexcept;
	Bitboard getPieces(PieceType type) const noexcept;
	Bitboard getPieces(Color color) const noexcept;

	// endregion Pieces

	template <Color C>
	Square getKingSq() const noexcept;
	Square getKingSq(Color color) const noexcept;

	bool isDrawn() const noexcept;
	Phase getPhase() const noexcept;

	void makeMove(Move move) noexcept;
	void undoMove() noexcept;
	void makeNullMove() noexcept;
	void undoNullMove() noexcept;

	template <PieceType>
	bool isAttacked(Color attackerColor, Square targetSquare, Bitboard blockers) const noexcept;
	bool isAttackedByAny(Color attackerColor, Square targetSquare, Bitboard blockers) const noexcept;
	bool isAttackedByAny(Color attackerColor, Square targetSquare) const noexcept;
	template <Color C>
	Bitboard generateKingAttackers() const noexcept;
	bool isSideInCheck() const noexcept;

private:
	void addPiece(Square square, Piece piece) noexcept;
	void movePiece(Square from, Square to) noexcept;
	void removePiece(Square square) noexcept;
    Bitboard findBlockers(Bitboard sliders, Square sq, Bitboard &pinners) const noexcept;

public:
    void computeCheckInfo() noexcept;
    void updatePieceList() noexcept;
	void updateNonPieceBitboards() noexcept;
	bool isLegal(Move move) const noexcept;
	Bitboard kingBlockers(Color color) const noexcept;

	std::string toString() const noexcept;

public:
    u64 zKey{};
    Bitboard occupied{};
    Bitboard kingAttackers{};
    std::array<std::array<Bitboard, PIECE_TYPE_NB>, COLOR_NB> pieces{};
    std::array<std::array<u8, 16>, 15> pieceList{};
    std::array<u8, 15> pieceCount{};

    std::array<Piece, SQUARE_NB> data{};

    short ply{};
    short npm{};
    Color colorToMove{};
    u8 castlingRights{};
    Square enPassantSq{};
    u8 fiftyMoveRule{};

    std::array<Bitboard, COLOR_NB> blockersForKing{};
    std::array<Bitboard, COLOR_NB> pinners{};
    std::array<Bitboard, PIECE_TYPE_NB> checkSquares{};

private:
    short historyPly{};
    std::array<UndoMove, MAX_MOVES> history{};
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

inline bool Board::canCastle(const Color color) const noexcept
{
	return color == BLACK ? canCastle<BLACK>() : canCastle<WHITE>();
}

inline Piece &Board::getPiece(const Square square) noexcept
{
	return data[square];
}

inline Bitboard &Board::getPieces(const Piece piece) noexcept
{
	return pieces[piece.color()][piece.type()];
}

inline Piece Board::getPiece(const Square square) const noexcept
{
	return data[square];
}

inline Bitboard Board::getPieces() const noexcept
{
	return occupied;
}

inline Bitboard Board::getPieces(const PieceType type, const Color color) const noexcept
{
	return pieces[color][type];
}

inline Bitboard Board::getPieces(const PieceType type) const noexcept
{
	return pieces[BLACK][type] | pieces[WHITE][type];
}

inline Bitboard Board::getPieces(const Color color) const noexcept
{
	return pieces[color][PieceType::NO_PIECE_TYPE];
}

template <Color C>
Square Board::getKingSq() const noexcept
{
	return toSquare(pieceList[Piece{ KING, C }][0]);
}

inline Square Board::getKingSq(const Color color) const noexcept
{
    return toSquare(pieceList[Piece{ KING, color }][0]);
}

template <PieceType P>
bool Board::isAttacked(const Color attackerColor, const Square targetSquare, Bitboard blockers) const noexcept
{
	static_assert(PAWN <= P);
	static_assert(P <= KING);

	const auto type = getPieces(P, attackerColor);

	if constexpr (P == PAWN)
	{
		const auto pawnAttacks = attackerColor
								 ? Attacks::pawnAttacks<WHITE>(type) : Attacks::pawnAttacks<BLACK>(type);

		return bool(pawnAttacks & Bitboard::fromSquare(targetSquare));
	} else if constexpr (P == KNIGHT)
		return bool(type & Attacks::knightAttacks(targetSquare));
	else if constexpr (P == BISHOP)
		return bool(type & Attacks::bishopAttacks(targetSquare, blockers));
	else if constexpr (P == ROOK)
		return bool(type & Attacks::rookAttacks(targetSquare, blockers));
	else if constexpr (P == QUEEN)
		return bool(type & Attacks::queenAttacks(targetSquare, blockers));
	else if constexpr (P == KING)
		return bool(type & Attacks::kingAttacks(targetSquare));

	return false;
}

template <Color C>
Bitboard Board::generateKingAttackers() const noexcept
{
	constexpr Color ColorAttacking = ~C;
	const Square kingSq = getKingSq<C>();
	assert(kingSq < SQUARE_NB);

	const auto queens = getPieces(QUEEN, ColorAttacking);
	const auto bishops = getPieces(BISHOP, ColorAttacking) | queens;
	const auto rooks = getPieces(ROOK, ColorAttacking) | queens;

	return (getPieces(PAWN, ColorAttacking) & Attacks::pawnAttacks<C>(Bitboard::fromSquare(kingSq)))
		   | (getPieces(KNIGHT, ColorAttacking) & Attacks::knightAttacks(kingSq))
		   | (getPieces(KING, ColorAttacking) & Attacks::kingAttacks(kingSq))
		   | (bishops & Attacks::bishopAttacks(kingSq, getPieces()))
		   | (rooks & Attacks::rookAttacks(kingSq, getPieces()));
}
