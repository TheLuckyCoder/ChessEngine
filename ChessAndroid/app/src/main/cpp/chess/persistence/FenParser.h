#pragma once

#include <sstream>

class Board;

class FenParser
{
	Board &board;

public:
	FenParser(Board &board);

	void parseFen(const std::string &fen) noexcept;
	std::string exportToFen() noexcept;

private:
	void parsePieces(std::istringstream &stream) noexcept;
};
