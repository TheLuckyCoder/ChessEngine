#include "SaveLoadJson.h"

#include "data/Board.h"
#include "data/pieces/Piece.h"

#include <iostream>

Board SaveLoadJson::load(std::string_view str)
{
	Board board;

	while (true)
	{
		const auto start = str.find('{');
		const auto end = str.find('}', start);

		if (start == std::string_view::npos || end == std::string_view::npos)
			break;

		loadPiece(board, str.substr(start + 1, end - start));

		str.remove_prefix(end);
	}

	return board;
}

std::string SaveLoadJson::save(const Board &board)
{
	std::ostringstream stream;
	std::boolalpha(stream);

	stream << '[';

	const auto pieces = board.getAllPieces();

	for (const auto &pair : pieces)
		savePiece(stream, pair.first, pair.second);

	auto str = stream.str();

	str[str.size() - 1] = ']'; // Replace the last ',' with a ']'

	return str;
}

void SaveLoadJson::loadPiece(Board &board, const std::string_view str)
{
	const auto isWhite = getValue<bool>(str, "white");
	const auto type = static_cast<Piece::Type>(getValue<int>(str, "type"));

	Piece piece(type, isWhite);
	piece.hasBeenMoved = getValue<bool>(str, "moved");

	board.data[getValue<short>(str, "x")][getValue<short>(str, "y")] = piece;
}

void SaveLoadJson::savePiece(std::ostringstream &stream, const Pos &pos, const Piece &piece)
{
	stream << '{'
		   << "\"x\":" << pos.x << ','
		   << "\"y\":" << pos.y << ','
		   << "\"type\":" << static_cast<int>(piece.type) << ','
		   << "\"white\":" << piece.isWhite << ','
		   << "\"moved\":" << piece.hasBeenMoved
		   << "},";
}
