#pragma once

#include <string>
#include <sstream>
#include <vector>

#include "../data/Pos.h"

class RootMove;
class Board;
class Piece;

class MovesPersistence final
{
	std::string m_Content;

public:
	MovesPersistence(std::string content);

	bool isPlayerWhite() const;
	std::vector<PosPair> getMoves() const;

	static std::string saveToString(const std::vector<RootMove> &movesHistory, bool isPlayerWhite);

private:
	static Pos getPos(std::string_view str);
	static void parsePosPair(std::vector<PosPair> &moves, std::string_view str);
	static void savePosPair(std::ostringstream &stream, const PosPair &pair);
};
