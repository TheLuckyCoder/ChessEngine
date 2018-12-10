#pragma once

#include <unordered_map>
#include <vector>

class Pos;
class Move;
class Piece;

// Class that keeps a 2D Array
class Board
{
public:
	Piece *data[8][8] = { {nullptr} };

	Board() = default;
	Board(Board &&board);
	Board(const Board &board);
	~Board();

	void initDefaultBoard();

	Piece *operator[](const Pos &pos);
	const Piece *operator[](const Pos &pos) const;

	int evaluate() const;
	std::unordered_map<Pos, Piece*> getAllPieces() const;
	std::vector<Move> listAllMoves(const bool isWhite) const;
};
