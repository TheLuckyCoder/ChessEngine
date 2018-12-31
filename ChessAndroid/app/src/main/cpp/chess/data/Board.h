#pragma once

#include <array>
#include <unordered_map>
#include <vector>

#include "pieces/Piece.h"

class Move;

// Class that keeps a 2D Array
class Board final
{
public:
	std::array<std::array<Piece, 8>, 8> data;
	std::uint64_t hash{};

	Board() = default;
	Board(Board &&board) noexcept;
	Board(const Board &board);
	~Board() = default;

	Board &operator=(Board &&other) noexcept;
	Board &operator=(const Board &other);
	Piece &operator[](const Pos &pos);
	const Piece &operator[](const Pos &pos) const;

	void initDefaultBoard();
	int evaluate() const;
	std::unordered_map<Pos, Piece> getAllPieces() const;
	std::vector<Move> listMoves(bool isWhite) const;
	std::vector<Move> listValidMoves(bool isWhite) const;
};
