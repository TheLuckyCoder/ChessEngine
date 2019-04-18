#include "MovesPersistence.h"

#include <algorithm>
#include <cctype>

#include "../data/Board.h"

std::vector<PosPair> MovesPersistence::load(std::string str)
{
	str.erase(std::remove_if(str.begin(), str.end(),
		[](const char c) { return std::isspace(c); }), str.end());
	std::vector<PosPair> moves;

	size_t prefix = 0;
	while (true)
	{
		const auto end = str.find(')', prefix);

		if (end == std::string_view::npos)
			break;

		parsePosPair(moves, str.substr(prefix + 1, end - prefix));

		prefix = end + 1;
	}

	return moves;
}

std::string MovesPersistence::save(const std::vector<RootMove> &movesHistory)
{
	std::ostringstream stream;

	for (const auto &moves : movesHistory)
		savePosPair(stream, std::make_pair(moves.start, moves.dest));

	return stream.str();
}

Pos MovesPersistence::getPos(const std::string_view str)
{
	const byte x = static_cast<byte>(str[0] - 48);
	const byte y = static_cast<byte>(str[2] - 48);

	return Pos(x, y);
}

void MovesPersistence::parsePosPair(std::vector<PosPair> &moves, std::string_view str)
{
	const auto selectedEnd = str.find(';');
	auto destEnd = str.find(')', selectedEnd + 1);

	if (destEnd == std::string_view::npos)
		destEnd = str.size() - 1;

	moves.emplace_back(getPos(str.substr(0, selectedEnd)), getPos(str.substr(selectedEnd + 1, destEnd)));
}

void MovesPersistence::savePosPair(std::ostringstream &stream, const PosPair &pair)
{
	stream << '('
		<< static_cast<int>(pair.first.x) << ',' << static_cast<int>(pair.first.y) << ';'
		<< static_cast<int>(pair.second.x) << ',' << static_cast<int>(pair.second.y) << ')';
}
