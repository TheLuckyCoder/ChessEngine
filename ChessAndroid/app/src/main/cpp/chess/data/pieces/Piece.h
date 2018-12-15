#pragma once

#include "../Board.h"
#include "../Pos.h"

class Piece
{
public:
	enum class Type : unsigned char
	{
		PAWN,
		KNIGHT,
		BISHOP,
		ROOK,
		QUEEN,
		KING
	};

	const Type type;
	const bool isWhite;
	bool hasBeenMoved;

	explicit Piece(const Type type, const bool isWhite)
		: type(type), isWhite(isWhite), hasBeenMoved(false) {}

	explicit Piece(const Piece *piece)
		: type(piece->type), isWhite(piece->isWhite), hasBeenMoved(piece->hasBeenMoved) {}

	virtual ~Piece() = default;

	std::vector<Pos> getPossibleMoves(Pos pos, const Board &board) const;
	int getPoints(int x, int y) const;
	bool hasSameColor(const Piece &other) const;

	bool operator==(const Piece &other) const;
	bool operator!=(const Piece &other) const;
protected:
	bool isMaximising() const;

	virtual void calculateMoves(Pos &pos, std::vector<Pos> &moves, const Board &board) const = 0;
	virtual int evaluatePiece(int x, int y) const = 0;
};
