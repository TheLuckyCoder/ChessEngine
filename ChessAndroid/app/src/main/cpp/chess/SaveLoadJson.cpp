#include "SaveLoadJson.h"

#include "data/Board.h"
#include "data/pieces/Pieces.h"

Board SaveLoadJson::load(std::string_view str)
{
	Board board{};

	unsigned long end;
	do {
		auto start = str.find('{');
		end = str.find('}', start);

		loadPiece(board, str.substr(start + 1, end));

		str.remove_suffix(end + 1); // + 2 ?
	} while (end != str.npos);

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

	std::string str = stream.str();

	str[str.size() - 1] = ']'; // Replace the last ',' with a ']'

	return str;
}

void SaveLoadJson::loadPiece(Board &board, std::string_view str)
{
	bool isWhite = getValue<bool>(str, "white");
	Piece::Type type = static_cast<Piece::Type>(getValue<unsigned char>(str, "type"));

	Piece *piece = [type, isWhite]() -> Piece* {
		switch (type)
		{
			case Piece::Type::PAWN:
				return new PawnPiece(isWhite);
			case Piece::Type::KNIGHT:
				return new KnightPiece(isWhite);
			case Piece::Type::BISHOP:
				return new BishopPiece(isWhite);
			case Piece::Type::ROOK:
				return new RookPiece(isWhite);
			case Piece::Type::QUEEN:
				return new QueenPiece(isWhite);
			case Piece::Type::KING:
				return new KingPiece(isWhite);
		}
	}();

	piece->hasBeenMoved = getValue<bool>(str, "moved");
	board.data[getValue<short>(str, "x")][getValue<short>(str, "y")] = piece;
}

void SaveLoadJson::savePiece(std::ostringstream &stream, const Pos &pos, const Piece *piece)
{
	stream << '{'
		   << "\"x\":" << pos.x << ','
		   << "\"y\":" << pos.y << ','
		   << "\"type\":" << static_cast<int>(piece->type) << ','
		   << "\"white\":" << piece->isWhite << ','
		   << "\"moved\":" << piece->hasBeenMoved
		   << "},";
}
