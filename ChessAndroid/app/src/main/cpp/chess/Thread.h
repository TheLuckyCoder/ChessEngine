#pragma once

#include <array>

class Thread
{
public:
	using Killers = std::array<std::array<unsigned int, MAX_DEPTH>, 2>;
	using History = std::array<std::array<byte, SQUARE_NB>, SQUARE_NB>;
	using Eval = std::array<int, MAX_DEPTH>;

	const std::size_t threadId;
	const bool mainThread;
	Killers killers{};
	History history{};
	Eval eval{};

	Thread(const std::size_t threadId, const bool mainThread)
		: threadId(threadId), mainThread(mainThread) {}
};
