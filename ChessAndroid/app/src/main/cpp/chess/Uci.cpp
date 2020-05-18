#include "Uci.h"

#include <iostream>
#include <thread>

#include "Stats.h"
#include "Tests.h"
#include "algorithm/Hash.h"
#include "algorithm/MoveGen.h"
#include "algorithm/Search.h"

std::thread Uci::searchThread{};
Board Uci::board{};

void Uci::init()
{
	Hash::init();
	Attacks::init();
}

void Uci::parsePosition(std::istringstream &is)
{
	std::string token;
	is >> token;

	if (token == "fen")
	{
		std::string fen;
		fen.reserve(64);

		while (is >> token && token != "moves")
		{
			fen += token;
			fen += ' ';
		}
		board.setToFen(fen);
	} else
	{
		// Load the default position anyhow
		board.setToStartPos();
	}

	if (is >> token && token == "moves")
	{
		while (true)
		{
			if (!(is >> token))
				break;
			const Move move = parseMove(board, token);
			if (!move.empty())
				board.makeMove(move);
		}
	}
	std::cout << "Loaded Pos" << std::endl;
}

void Uci::parseGo(std::istringstream &is)
{
	int depth = MAX_DEPTH;
	int movesToGo = 35;
	int moveTime = -1;
	int time = -1;
	bool timeSet = false;
	int inc = 0;
	bool infinite = false;

	std::string token;

	while (is >> token)
	{
		if (board.colorToMove == WHITE)
		{
			if (token == "wtime")
				is >> time;
			else if (token == "winc")
				is >> inc;
		} else
		{
			if (token == "btime")
				is >> time;
			else if (token == "binc")
				is >> inc;
		}

		if (token == "infinite")
			infinite = true;
		else if (token == "movestogo")
			is >> movesToGo;
		else if (token == "movetime")
			is >> moveTime;
		else if (token == "depth")
			is >> depth;
	}

	if (moveTime != -1)
	{
		time = moveTime;
		movesToGo = 1;
	}

	if (time != -1)
	{
		timeSet = true;
		time /= movesToGo;
		time -= 50;
		time += inc / 2;
	}

	if (timeSet)
		time = std::max(5, time);

	if (infinite)
	{
		time = 0;
		timeSet = false;
	}

	std::cout << "time: " << time << " depth: " << depth << " timeSet: " << std::boolalpha << timeSet << std::endl;

	const auto searchTime = timeSet ? static_cast<size_t>(time) : 0ul;
	const Settings settings{ static_cast<size_t>(depth), 1, 0, true, searchTime };

	if (searchThread.joinable())
		searchThread.detach();
	Stats::resetStats();

	searchThread = std::thread(Search::findBestMove, board, settings);
}

void printEngineInfo()
{
	std::cout << "id name TestEngine\n"
			  << "id author TheLuckyCoder\n"
			  << "uciok" << std::endl;
}

void Uci::loop()
{
	std::string line;
	std::string token;
	line.reserve(2048);
	token.reserve(64);

	printEngineInfo();

	board.setToStartPos();

	while (std::getline(std::cin, line))
	{
		std::istringstream is(line);
		bool quit{};

		if (line.empty())
			continue;

		is >> token;

		if (token == "isready")
			std::cout << "readyok\n";
		else if (token == "position" || token == "pos")
			parsePosition(is);
		else if (token == "go")
			parseGo(is);
		else if (token == "uci")
			printEngineInfo();
		else if (token == "ucinewgame")
		{
			board.setToStartPos();
			Search::clearAll();
		} else if (token == "stop")
		{
			Search::stopSearch();
			searchThread.join();
			std::cout << "Joined Thread" << std::endl;
		} else if (token == "quit")
			quit = true;

			// Non-Uci Commands
		else if (token == "evaltest")
		{
			const auto results = Tests::runEvaluationTests();
			if (results.empty())
				std::cout << "Test Completed Successfully";
			else
				std::cout << results;
			std::cout << std::endl;
		} else if (token == "perft")
			Tests::runPerftTests();

		if (quit)
			break;

		line.clear();
	}
}
