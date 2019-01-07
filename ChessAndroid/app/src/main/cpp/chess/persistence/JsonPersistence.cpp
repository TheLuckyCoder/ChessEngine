#include "JsonPersistence.h"

#include <cctype>

#include "../data/Board.h"
#include "../data/pieces/Piece.h"
#include "../minimax/Hash.h"

Board JsonPersistence::load(std::string str)
{
	str.erase(std::remove_if(str.begin(), str.end(),
		[](const char c) { return std::isspace(c); }), str.end());
	Board board;

	unsigned long prefix = 0;
	while (true)
	{
		const auto start = str.find('{', prefix);
		const auto end = str.find('}', start);

		if (start == std::string_view::npos || end == std::string_view::npos)
			break;

		parsePiece(board, str.substr(start + 1, end - start));

		prefix = end;
	}

	board.hash = Hash::compute(board);

	return board;
}

std::string JsonPersistence::save(const Board &board)
{
	std::ostringstream stream;

	stream << '[';

	const auto pieces = board.getAllPieces();

	for (const auto &pair : pieces)
		savePiece(stream, pair.first, pair.second);

	auto str = stream.str();

	str[str.size() - 1] = ']'; // Replace the last ',' with a ']'

	return str;
}

void JsonPersistence::parsePiece(Board &board, const std::string_view str)
{
	const auto isWhite = getValue<bool>(str, "white", false);
	const auto type = static_cast<Piece::Type>(getValue<int>(str, "type"));
	const auto moved = getValue<bool>(str, "moved", false);

	board.data[getValue<short>(str, "x")][getValue<short>(str, "y")] = Piece(type, isWhite, moved);
}

void JsonPersistence::savePiece(std::ostringstream &stream, const Pos &pos, const Piece &piece)
{
	stream << '{'
		<< "\"x\":" << pos.x << ','
		<< "\"y\":" << pos.y << ','
		<< "\"type\":" << static_cast<int>(piece.type) << ',';

	if (piece.isWhite)
		stream << "\"white\":true,";

	if (piece.moved)
		stream << "\"moved\":true,";

	stream << "},";
}
