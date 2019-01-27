#include "MovesPersistence.h"

#include <algorithm>
#include <cctype>

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

std::string MovesPersistence::save(const std::vector<PosPair> &movesHistory)
{
	std::ostringstream stream;

	for (const auto &pair : movesHistory)
		savePosPair(stream, pair);

	return stream.str();
}

Pos MovesPersistence::getPos(const std::string_view str)
{
	const auto x = static_cast<short>(str[0]);
	const auto y = static_cast<short>(str[2]);

	return Pos(static_cast<byte>(x), static_cast<byte>(y));
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
