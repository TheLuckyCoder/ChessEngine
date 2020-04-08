#pragma once

#include <sstream>

class Board;

class FenParser
{
	Board &board;

public:
	FenParser(Board &board);

	void parseFen(const std::string &fen);
	std::string exportToFen();

private:
	void parsePieces(std::istringstream &stream) const;
};
