#pragma once

#include <string>
#include <sstream>
#include <type_traits>

class Board;
class Piece;
class Pos;

class SaveLoadJson final {
private:
	SaveLoadJson() = delete;

public:
	static Board load(std::string_view str);
	static std::string save(const Board &board);

private:
	template<typename T>
	static T getValue(const std::string_view &str, std::string_view find)
	{
		auto start = str.find(find) + find.length() + 2;

		auto end = str.find(',', start);
		auto valueString = str.substr(start, end);

		if constexpr (std::is_same_v<T, std::string>)
			return valueString;
		else if (std::is_same_v<T, bool>)
			return valueString == "true";
		else if (std::is_same_v<T, char> || std::is_same_v<T, unsigned char>)
			return valueString.front();
		else
		{
			T value;
			std::stringstream ss;
			ss << valueString;
			ss >> value;
			return value;
		}
	}

	static void loadPiece(Board &board, std::string_view str);
	static void savePiece(std::ostringstream &stream, const Pos &pos, const Piece *piece);
};
