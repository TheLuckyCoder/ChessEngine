#pragma once

#include "../data/Piece.h"

class MoveOrdering
{
	constexpr static int victimScore[] = { 0, 100, 200, 300, 400, 500, 600 };

public:
	inline static int MvvLvaScore[7][7];

	static void init()
	{
		static bool initialized = false;
		if (initialized) return;
		initialized = true;

		for (int attacker = PAWN; attacker <= KING; ++attacker)
		{
			for (int victim = PAWN; victim <= KING; ++victim)
			{
				MvvLvaScore[victim][attacker] = victimScore[victim] + 6 - (victimScore[attacker] / 100);
			}
		}
	}
};
