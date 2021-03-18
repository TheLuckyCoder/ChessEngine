#include "MovesPersistence.h"

#include <algorithm>
#include <sstream>

MovesPersistence::MovesPersistence(std::string content)
	: _content(std::move(content))
{
	_content.erase(std::remove_if(_content.begin(), _content.end(),
			[](const char c) { return std::isspace(c); }), _content.end());
}

bool MovesPersistence::isPlayerWhite() const
{
	return _content[0] != 'B';
}

std::vector<Move> MovesPersistence::getMoves() const
{
	std::vector<Move> moves;
	moves.reserve(MAX_MOVES / 2);

	size_t prefix = 1u;
	while (true)
	{
		const auto end = _content.find(';', prefix);

		if (end == std::string_view::npos)
			break;

		const auto moveStr = _content.substr(prefix, end - prefix);

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

	for (const Move &move : moves)
		stream << move.getContents() << ';';

	return stream.str();
}
