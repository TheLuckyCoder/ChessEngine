#pragma once

#include <string>
#include <vector>

#include "../data/Move.h"

class MovesPersistence final
{
public:
	explicit MovesPersistence(std::string content);

	bool isPlayerWhite() const;
	std::vector<Move> getMoves() const;

	static std::string saveToString(const std::vector<Move> &moves, bool isPlayerWhite);

private:
	std::string _content;
};
