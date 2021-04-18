#pragma once

#include "../Board.h"

namespace PolyBook
{
	void initBook(const std::string &bookPath);
	bool initialized() noexcept;
	void clearBook();

	u64 getKeyFromBoard(const Board &board) noexcept;
	Move getBookMove(const Board &board) noexcept;
}
