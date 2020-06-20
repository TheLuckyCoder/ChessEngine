#pragma once

#include <sstream>

class Board;

class FenParser
{
public:
	explicit FenParser(Board &board);

	bool parseFen(const std::string &fen);
	std::string exportToFen();

private:
	void parsePieces(std::istringstream &stream);

	Board &_board;
};
