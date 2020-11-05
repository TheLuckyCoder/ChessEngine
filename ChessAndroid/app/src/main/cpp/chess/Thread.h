#pragma once

#include <array>

class Thread
{
public:
	using Killers = std::array<std::array<u32, MAX_DEPTH>, 2>;
	using History = std::array<std::array<u8, SQUARE_NB>, SQUARE_NB>;
	using Eval = std::array<int, MAX_DEPTH>;

	const usize threadId;
	const bool mainThread;
	Killers killers{};
	History history{};
	Eval eval{};

	usize nodesCount{};

	Thread(const std::size_t threadId, const bool mainThread)
		: threadId(threadId), mainThread(mainThread) {}

	void clear() noexcept
	{
		killers.fill({});
		history.fill({});
		eval.fill({});
	}
};
