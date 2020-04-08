#include "MovesPersistence.h"

#include <charconv>
#include <sstream>

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

std::vector<Move> MovesPersistence::getMoves() const
{
	std::vector<Move> moves;
	moves.reserve(100);

	size_t prefix = 1u;
	while (true)
	{
		const auto end = m_Content.find(';', prefix);

		if (end == std::string_view::npos)
			break;

		const auto moveStr = m_Content.substr(prefix + 1, end - prefix);

		const auto moveContents = std::stoul(moveStr);
		moves.emplace_back(static_cast<unsigned int>(moveContents), 0);

		prefix = end + 1;
	}

	return moves;
}

std::string MovesPersistence::saveToString(const std::vector<Move> &moves, const bool isPlayerWhite)
{
	std::ostringstream stream;
	stream << (isPlayerWhite ? 'W' : 'B');

	for (const Move move : moves)
		stream << move.getContents() << ';';

	return stream.str();
}
