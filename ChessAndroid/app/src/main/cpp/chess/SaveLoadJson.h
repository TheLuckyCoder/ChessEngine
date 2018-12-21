#pragma once

#include <string>
#include <sstream>

class Board;
class Piece;
class Pos;

class SaveLoadJson final {
public:
	SaveLoadJson() = delete;

	static Board load(std::string str);
	static std::string save(const Board &board);

private:
	template<typename T>
	static T getValue(const std::string_view &str, std::string_view key)
	{
		const auto start = str.find(key) + key.length() + 2;

		const auto end = str.find_first_of(",}", start) - start;
		const auto valueString = str.substr(start, end);

		if constexpr (std::is_same_v<T, std::string>)
			return std::string(valueString);
		else if (std::is_same_v<T, bool>)
			return valueString == "true";
		else
		{
			T value;
			std::stringstream ss;
			ss << std::string(valueString);
			ss >> value;
			return value;
		}
	}

	static void parsePiece(Board &board, std::string_view str);
	static void savePiece(std::ostringstream &stream, const Pos &pos, const Piece &piece);
};
