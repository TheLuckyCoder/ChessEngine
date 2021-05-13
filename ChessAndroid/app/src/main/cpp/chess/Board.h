#pragma once

#include <array>
#include <string>

#include "Piece.h"
#include "Move.h"
#include "algorithm/Attacks.h"

class BoardState final
{
public:
	u64 zKey{};

	Bitboard kingAttackers{};
	std::array<Bitboard, COLOR_NB> kingBlockers{};
	std::array<Bitboard, COLOR_NB> kingPinners{};
	std::array<Bitboard, PIECE_TYPE_NB> possibleCheckSquares{};

	u32 moveContents{};
	u8 castlingRights{};
	Square enPassantSq = SQ_NONE;
	u8 fiftyMoveRule{};

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

	Square getKingSq(Color color) const noexcept;

	// endregion Pieces

	// region State

	u64 zKey() const noexcept;
	Square getEnPassantSq() const noexcept;
	CastlingRights getCastlingRights() const noexcept;

	bool isDrawn() const noexcept;
	Phase getPhase() const noexcept;

	// endregion State

	// region Move functions

	void makeMove(Move move) noexcept;
	void makeMove(Move move, bool moveGivesCheck) noexcept;
	void undoMove() noexcept;
	void makeNullMove() noexcept;
	void undoNullMove() noexcept;
	bool doesMoveGiveCheck(Move move) const noexcept;
	bool isMoveLegal(const Move move) const noexcept;

	// endregion

	Bitboard generateAttackers(Color attackerColor, Square sq, Bitboard blockers) const noexcept;
	Bitboard generateAttackers(Square sq, Bitboard blockers) const noexcept;
	Bitboard generateAttackers(Square sq) const noexcept;
	bool isSideInCheck() const noexcept;

private:
	void addPiece(Square square, Piece piece) noexcept;
	void movePiece(Square from, Square to) noexcept;
	void removePiece(Square square) noexcept;
    Bitboard findBlockers(const Bitboard sliders, const Square sq, Bitboard &pinners) const noexcept;

public:
    void computeCheckInfo() noexcept;
    void updatePieceList() noexcept;
	void updateNonPieceBitboards() noexcept;
	Bitboard getKingAttackers() const noexcept;
	Bitboard getKingBlockers(Color color) const noexcept;

	std::string toString() const noexcept;

public:
    Bitboard occupied{};
    std::array<std::array<Bitboard, PIECE_TYPE_NB>, COLOR_NB> pieces{};
    std::array<u8, 15> pieceCount{};

    std::array<Piece, SQUARE_NB> data{};

    BoardState state{};

    i16 ply{};
	i16 npm{};
    Score psq{};
    Color colorToMove{};

private:
    short historyPly{};
    std::array<BoardState, MAX_MOVES> history{};
};

template <Color C>
bool Board::canCastle() const noexcept
{
	return state.castlingRights & (C == BLACK ? CASTLE_BLACK_BOTH : CASTLE_WHITE_BOTH);
}

template <Color C>
bool Board::canCastleKs() const noexcept
{
	return state.castlingRights & (C == BLACK ? CASTLE_BLACK_KING : CASTLE_WHITE_KING);
}

template <Color C>
bool Board::canCastleQs() const noexcept
{
	return state.castlingRights & (C == BLACK ? CASTLE_BLACK_QUEEN : CASTLE_WHITE_QUEEN);
}

template <Color C>
bool Board::isCastled() const noexcept
{
	return state.castlingRights & (C == BLACK ? CASTLED_BLACK : CASTLED_WHITE);
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

inline Square Board::getKingSq(const Color color) const noexcept
{
    return getPieces(KING, color).bitScanForward();
}

inline u64 Board::zKey() const noexcept
{
	return state.zKey;
}

inline Square Board::getEnPassantSq() const noexcept
{
	return state.enPassantSq;
}

inline CastlingRights Board::getCastlingRights() const noexcept
{
	return CastlingRights(state.castlingRights);
}

inline Bitboard Board::getKingAttackers() const noexcept
{
	return state.kingAttackers;
}

inline Bitboard Board::getKingBlockers(const Color color) const noexcept
{
	return state.kingBlockers[color];
}
