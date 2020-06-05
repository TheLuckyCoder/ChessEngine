#pragma once

#include <string>
#include <sstream>
#include <thread>

#include "Board.h"

class Uci
{
	static std::thread searchThread;
	static Board board;

public:
	static void init();
	static void loop();

private:
	static void printEngineInfo();
	static void parsePosition(std::istringstream &is);
	static void parseGo(std::istringstream &is);
};