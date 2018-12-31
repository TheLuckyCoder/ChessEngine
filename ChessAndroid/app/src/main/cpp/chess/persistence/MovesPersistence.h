#pragma once

#include <string>
#include <sstream>
#include <vector>

#include "../data/Pos.h"

class Board;
class Piece;

class MovesPersistence final {
public:
	MovesPersistence() = delete;

	static std::vector<PosPair> load(std::string str);
	static std::string save(const std::vector<PosPair> &movesHistory);

private:
	static Pos getPos(std::string_view str);
	static void parsePosPair(std::vector<PosPair> &moves, std::string_view str);
	static void savePosPair(std::ostringstream &stream, const PosPair &pair);
};
