#pragma once

#include <string>

#include "Defs.h"
#include "Pos.h"

class Move final
{
public:
	enum Flag
	{
		CAPTURE = 1 << 1, // The move is a capture
		PROMOTION = 1 << 2, // The move is a promotion
		KSIDE_CASTLE = 1 << 3, // The move is a king side castle
		QSIDE_CASTLE = 1 << 4, // The move is a queen side castle
		DOUBLE_PAWN_PUSH = 1 << 5, // The move is a double pawn push
		EN_PASSANT = 1 << 6 // The move is an en passant capture (Do not set the CAPTURE flag additionally)
	};

	Move() = default;

	explicit constexpr Move(const unsigned int move, const int score = 0) noexcept
		: _move(move), _score(score)
	{
	}

	constexpr Move(const byte from, const byte to, const PieceType piece) noexcept
		: _move(((to & 0x3F) << 15u) | ((from & 0x3F) << 9u) | (piece & 0x7))
	{
	}

	constexpr Move(const byte from, const byte to, const PieceType piece,
				   const unsigned int flags) noexcept
		: _move(((flags & 0x7F) << 22u) | ((to & 0x3F) << 15u) | ((from & 0x3F) << 9u) | (piece & 0x7))
	{
	}

	constexpr bool empty() const noexcept
	{
		return !static_cast<bool>(_move);
	}

	constexpr unsigned int getContents() const noexcept
	{
		return _move;
	}

	constexpr int getScore() const noexcept
	{
		return _score;
	}

	constexpr void setScore(const int score) noexcept
	{
		this->_score = score;
	}

	constexpr PieceType piece() const noexcept
	{
		return static_cast<PieceType>(_move & 7u);
	}

	constexpr PieceType capturedPiece() const noexcept
	{
		return static_cast<PieceType>((_move >> 3u) & 7u);
	}

	constexpr void setCapturedPiece(const PieceType type) noexcept
	{
		constexpr unsigned int mask = 0x7 << 3u;
		_move = (_move & ~mask) | ((type << 3u) & mask);
	}

	constexpr PieceType promotedPiece() const noexcept
	{
		return static_cast<PieceType>((_move >> 6u) & 7u);
	}

	constexpr void setPromotedPiece(const PieceType type) noexcept
	{
		constexpr unsigned int mask = 0x7 << 6u;
		_move = (_move & ~mask) | ((type << 6u) & mask);
	}

	constexpr byte from() const noexcept
	{
		return (_move >> 9u) & 0x3F;
	}

	constexpr byte to() const noexcept
	{
		return (_move >> 15u) & 0x3F;
	}

	constexpr unsigned int flags() const noexcept
	{
		return (_move >> 22u) & 0x7F;
	}

	constexpr void setFlags(const unsigned int flags) noexcept
	{
		constexpr unsigned int mask = 0x7F << 22u;
		_move = (_move & ~mask) | ((flags << 22u) & mask);
	}

	constexpr bool isAdvancedPawnPush() const noexcept
	{
		const byte y = row(to());
		return piece() == PAWN && (y == 7 || y == 2);
	}

	constexpr bool operator==(const Move &rhs) const noexcept
	{
		return _move == rhs._move;
	}

	constexpr bool operator!=(const Move &rhs) const noexcept
	{
		return _move != rhs._move;
	}

	std::string toString() const
	{
		std::string str;
		str.reserve(5);

		const Pos fromPos{ from() };
		const Pos toPos{ to() };

		{
			const PieceType p = piece();
			char pChar = 'K';
			if (p == PAWN)
				pChar = 'P';
			else if (p == KNIGHT)
				pChar = 'N';
			else if (p == BISHOP)
				pChar = 'B';
			else if (p == ROOK)
				pChar = 'R';
			else if (p == QUEEN)
				pChar = 'Q';

			str += pChar;
		}

		str += 'a' + fromPos.x;
		str += '1' + fromPos.y;
		str += 'a' + toPos.x;
		str += '1' + toPos.y;

		if (flags() & Flag::PROMOTION)
		{
			const PieceType promoted = promotedPiece();
			str.erase(0, 1);
			char p = 'Q';
			if (promoted == ROOK)
				p = 'R';
			else if (promoted == BISHOP)
				p = 'B';
			else if (promoted == KNIGHT)
				p = 'N';

			str += p;
		}

		return str;
	}

private:
	/*
	 * Bits 0 to 2 (3 bits) store the 'moved' piece type
	 * Bits 3 to 5 (3 bits) store the 'captured' piece type
	 * Bits 6 to 8 (3 bits) store the 'promoted' piece type
	 * Bits 9 to 15 (6 bits) store the 'from' square
	 * Bits 16 to 22 (6 bits) store the 'to' square
	 * Bits 23 to 30 (7 bits) store the 'flags'
	 */
	unsigned int _move{};
	int _score{};
};
