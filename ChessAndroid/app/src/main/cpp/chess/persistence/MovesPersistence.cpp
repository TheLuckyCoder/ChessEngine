#include "MovesPersistence.h"

#include <algorithm>
#include <cctype>

std::vector<PosPair> MovesPersistence::load(std::string str)
{
	str.erase(std::remove_if(str.begin(), str.end(),
		[](const char c) { return std::isblank(c); }), str.end());
	std::vector<PosPair> moves;

	unsigned long prefix{};
	while (true)
	{
		const auto end = str.find('\n', prefix);

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

	auto str = stream.str();
	str.pop_back();
	return str;
}

Pos MovesPersistence::getPos(const std::string_view str)
{
	const auto x = static_cast<short>(str[0]);
	const auto y = static_cast<short>(str[2]);

	return Pos(x, y);
}

void MovesPersistence::parsePosPair(std::vector<PosPair> &moves, std::string_view str)
{
	const auto selectedEnd = str.find(';');
	const auto destEnd = str.find('\n', selectedEnd + 1);

	moves.emplace_back(getPos(str.substr(0, selectedEnd)), getPos(str.substr(selectedEnd + 1, destEnd)));
}

void MovesPersistence::savePosPair(std::ostringstream &stream, const PosPair &pair)
{
	stream << pair.first.x << ',' << pair.first.y << ';'
		<< pair.second.x << ',' << pair.second.y << '\n';
}