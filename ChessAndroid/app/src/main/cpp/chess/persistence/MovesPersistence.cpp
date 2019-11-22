#include "MovesPersistence.h"

#include <algorithm>
#include <cctype>
#include <charconv>

#include "../data/Board.h"

MovesPersistence::MovesPersistence(std::string content)
	: m_Content(std::move(content))
{
	m_Content.erase(std::remove_if(m_Content.begin(), m_Content.end(),
			[](const char c) { return std::isspace(c); }), m_Content.end());
}

bool MovesPersistence::isPlayerWhite() const
{
	return m_Content[0] != 'B';
}

std::vector<std::pair<byte, byte>> MovesPersistence::getMoves() const
{
	std::vector<std::pair<byte, byte>> moves;
	moves.reserve(20);

	size_t prefix = 1;
	while (true)
	{
		const auto end = m_Content.find(')', prefix);

		if (end == std::string_view::npos)
			break;

		parsePosPair(moves, m_Content.substr(prefix + 1, end - prefix));

		prefix = end + 1;
	}

	return moves;
}

std::string MovesPersistence::saveToString(const std::vector<RootMove> &movesHistory, const bool isPlayerWhite)
{
	std::ostringstream stream;
	stream << (isPlayerWhite ? 'W' : 'B');

	for (const RootMove &moves : movesHistory)
		savePosPair(stream, std::make_pair(moves.startSq, moves.destSq));

	return stream.str();
}

byte MovesPersistence::getSquare(std::string_view str)
{
	int value = -1;
	std::from_chars(str.data(), str.data() + str.size(), value);

	return static_cast<byte>(value);
}

void MovesPersistence::parsePosPair(std::vector<std::pair<byte, byte>> &moves, std::string_view str)
{
	const auto selectedEnd = str.find(';');
	auto destEnd = str.find(')', selectedEnd + 1);

	if (destEnd == std::string_view::npos)
		destEnd = str.size() - 1;

	moves.emplace_back(getSquare(str.substr(0, selectedEnd)),
					   getSquare(str.substr(selectedEnd + 1, destEnd)));
}

void MovesPersistence::savePosPair(std::ostringstream &stream, const std::pair<byte, byte> &pair)
{
	if (pair.first < 64 && pair.second < 64)
	{
		stream << '('
			   << static_cast<int>(pair.first)
			   << ';'
			   << static_cast<int>(pair.second)
			   << ')';
	}
}
