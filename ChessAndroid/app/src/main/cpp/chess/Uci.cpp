#include "Uci.h"

#include <iostream>

#include "Stats.h"
#include "Tests.h"
#include "Zobrist.h"
#include "algorithm/Search.h"

void Uci::init()
{
	Zobrist::init();
	board.setToStartPos();
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
			printEngineInfo();
		} else if (token == "setoption")
		{
			is >> token;

			if (token == "threads")
			{
				is >> _threadCount;
				_threadCount = std::clamp<usize>(_threadCount, 1u, 64u);

				std::cout << "Thread Count has been set to " << _threadCount << std::endl;
			}
		} else if (token == "debug")
		{
			is >> token;

			if (token == "on")
			{
				Stats::setEnabled(true);
				std::cout << "Debug stats have been turned on";
			} else if (token == "off")
			{
				Stats::setEnabled(false);
				std::cout << "Debug stats have been turned off";
			} else {
				std::cout << (Stats::isEnabled() ? "Debug stats are on" : "Debug stats are off");
			}

			std::cout << std::endl;
		} else if (token == "stop")
		{
			Search::stopSearch();
			_searchThread.join();
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
		timeSet = true;
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
	const Settings settings{ depth, _threadCount, 100, true, searchTime };

	if (_searchThread.joinable())
		_searchThread.detach();
	Stats::resetStats();

	_searchThread = std::thread(Search::findBestMove, board, settings);
}

void Uci::parsePosition(std::istringstream &is)
{
	std::string token;
	is >> token;

	if (token == "fen")
	{
		std::string fen;
		fen.reserve(92);

		while (is >> token && !(token == "moves"))
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

void Uci::printEngineInfo()
{
	std::cout << "id name TestEngine\n"
			  << "id author TheLuckyCoder\n"
			  << "uciok" << std::endl;
}
