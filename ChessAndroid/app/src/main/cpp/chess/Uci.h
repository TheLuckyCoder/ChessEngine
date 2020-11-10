#pragma once

#include <string>
#include <sstream>
#include <thread>

#include "Board.h"

class Uci
{
	inline static std::thread _searchThread{};
	inline static usize _threadCount{ std::thread::hardware_concurrency() - 1 };
	inline static Board board{};

public:
	static void init();
	static void loop();

private:
	static void printEngineInfo();
	static void parsePosition(std::istringstream &is);
	static void parseGo(std::istringstream &is);
};
