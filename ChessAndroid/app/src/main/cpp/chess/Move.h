#pragma once

#include <string>

#include "Defs.h"

class Move final
{
public:
	class Flags final
	{
	public:
		enum Internal
		{
			CAPTURE = 1 << 1, // The move is a capture
			PROMOTION = 1 << 2, // The move is a promotion
			KSIDE_CASTLE = 1 << 3, // The move is a king side castle
			QSIDE_CASTLE = 1 << 4, // The move is a queen side castle
			CASTLE = KSIDE_CASTLE | QSIDE_CASTLE,
			DOUBLE_PAWN_PUSH = 1 << 5, // The move is a double pawn push
			EN_PASSANT = 1 << 6 // The move is an en passant capture (Do not set the CAPTURE flag too)
		};

	public:
		constexpr Flags(const byte flags) noexcept : _flags(flags & 0x7F) {}

		constexpr bool capture() const noexcept { return _flags & Internal::CAPTURE; }

		constexpr bool promotion() const noexcept { return _flags & Internal::PROMOTION; }

		constexpr bool kSideCastle() const noexcept { return _flags & Internal::KSIDE_CASTLE; }

		constexpr bool qSideCastle() const noexcept { return _flags & Internal::QSIDE_CASTLE; }

		constexpr bool castle() const noexcept { return _flags & Internal::CASTLE; }

		constexpr bool doublePawnPush() const noexcept { return _flags & Internal::DOUBLE_PAWN_PUSH; }

		constexpr bool enPassant() const noexcept { return _flags & Internal::EN_PASSANT; }

	private:
		byte _flags;
	};

	Move() = default;

	explicit constexpr Move(const unsigned int move, const int score = 0) noexcept
		: _move(move), _score(score)
	{
	}

	constexpr Move(const byte from, const byte to, const PieceType piece) noexcept
		: _move(((to & 0x3Fu) << 15u) | ((from & 0x3Fu) << 9u) | (piece & 0x7))
	{
	}

	constexpr Move(const byte from, const byte to, const PieceType piece,
				   const byte flags) noexcept
		: _move(((flags & 0x7Fu) << 22u) | ((to & 0x3Fu) << 15u) | ((from & 0x3Fu) << 9u) | (piece & 0x7u))
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
		_score = score;
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
		constexpr unsigned Mask = 0x7u << 3u;
		_move = (_move & ~Mask) | ((type << 3u) & Mask);
	}

	constexpr PieceType promotedPiece() const noexcept
	{
		return static_cast<PieceType>((_move >> 6u) & 7u);
	}

	constexpr void setPromotedPiece(const PieceType type) noexcept
	{
		constexpr unsigned Mask = 0x7u << 6u;
		_move = (_move & ~Mask) | ((type << 6u) & Mask);
	}

	constexpr byte from() const noexcept
	{
		return (_move >> 9u) & 0x3F;
	}

	constexpr byte to() const noexcept
	{
		return (_move >> 15u) & 0x3F;
	}

	constexpr Flags flags() const noexcept
	{
		return Flags(static_cast<byte>(_move >> 22u));
	}

	constexpr void setFlags(const byte flags) noexcept
	{
		constexpr unsigned Mask = 0x7Fu << 22u;
		_move = (_move & ~Mask) | ((flags << 22u) & Mask);
	}

	constexpr bool isTactical() const noexcept
	{
		const auto f = flags();
		return f.capture() | f.promotion();
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

	std::string toString(const bool showPiece = false) const
	{
		std::string str;
		str.reserve(5);

		const byte fromSq{ from() };
		const byte toSq{ to() };

		if (showPiece)
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

		str += 'a' + col(fromSq);
		str += '1' + row(fromSq);
		str += 'a' + col(toSq);
		str += '1' + row(toSq);

		if (flags().promotion())
		{
			const PieceType promoted = promotedPiece();
			if (showPiece)
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
