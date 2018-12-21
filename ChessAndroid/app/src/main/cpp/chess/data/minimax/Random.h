#pragma once

#include <cstdint>

struct PCG
{
	constexpr static std::uint64_t seed()
	{
		uint64_t shifted = 0;

		for (const auto c : __TIME__)
		{
			shifted <<= 8;
			shifted |= c;
		}

		return shifted;
	}

	struct pcg32_random_t { std::uint64_t state = 0; std::uint64_t inc = seed(); };
	pcg32_random_t rng;

	constexpr auto operator()()
	{
		return pcg32_random_r();
	}

private:
	constexpr std::uint64_t pcg32_random_r()
	{
		const auto oldState = rng.state;
		// Advance internal state
		rng.state = oldState * 6364136223846793005ULL + (rng.inc | 1);
		// Calculate output function (XSH RR), uses old state for max ILP
		const std::uint64_t xorShifted = ((oldState >> 18u) ^ oldState) >> 27u;
		const std::uint64_t rot = oldState >> 59u;
		return (xorShifted >> rot) | (xorShifted << (rot & 31));
	}
};
