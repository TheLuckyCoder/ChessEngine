#pragma once

#include <array>
#include <cstdint>

using U64 = std::uint64_t;

namespace Bitboard
{
	namespace
	{
		constexpr std::array<U64, 64> init()
		{
			std::array<U64, 64> array{};
			array[0] = 1ull;

			for (auto i = 1u; i < 64u; ++i)
				array[i] = array[i - 1] << 1;

			return array;
		}
	}

	constexpr std::array<U64, 64> positions = init();
}
