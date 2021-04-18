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
			CAPTURE = 1, // The move is a capture
			PROMOTION = 1 << 1, // The move is a promotion
			KSIDE_CASTLE = 1 << 2, // The move is a king SideKey castle
			QSIDE_CASTLE = 1 << 3, // The move is a queen SideKey castle
			DOUBLE_PAWN_PUSH = 1 << 4, // The move is a double pawn push
			EN_PASSANT = 1 << 5, // The move is an en passant capture (Do not set the CAPTURE flag too)
			PV_MOVE = 1 << 6, // The move is a PV Move
		};

	public:
		explicit constexpr Flags(const u8 flags) noexcept: _flags(flags & 0x7F) {}

		constexpr u8 getContents() const noexcept { return _flags; }

		constexpr bool capture() const noexcept { return _flags & Internal::CAPTURE; }

		constexpr bool promotion() const noexcept { return _flags & Internal::PROMOTION; }

		constexpr bool kSideCastle() const noexcept { return _flags & Internal::KSIDE_CASTLE; }

		constexpr bool qSideCastle() const noexcept { return _flags & Internal::QSIDE_CASTLE; }

		constexpr bool doublePawnPush() const noexcept { return _flags & Internal::DOUBLE_PAWN_PUSH; }

		constexpr bool enPassant() const noexcept { return _flags & Internal::EN_PASSANT; }

		constexpr bool pvMove() const noexcept { return _flags & Internal::PV_MOVE; }

	private:
		u8 _flags;
	};

	Move() = default;

	explicit constexpr Move(const unsigned int move, const int score = 0) noexcept
		: _move(move), _score(score)
	{
	}

	constexpr Move(const u8 from, const u8 to, const PieceType piece, const u8 flags = {}) noexcept
		: _move(((from & FROM_MASK) << FROM_SHIFT)
				| ((to & TO_MASK) << TO_SHIFT)
				| ((piece & MOVED_MASK) << MOVED_SHIFT)
				| ((flags & FLAGS_MASK) << FLAGS_SHIFT))
	{
	}

	constexpr bool empty() const noexcept { return !static_cast<bool>(_move); }

	constexpr u32 getContents() const noexcept { return _move; }

	constexpr i32 getScore() const noexcept { return _score; }

	constexpr void setScore(const int score) noexcept
	{
		_score = score;
	}

	constexpr PieceType piece() const noexcept
	{
		return static_cast<PieceType>((_move >> MOVED_SHIFT) & MOVED_MASK);
	}

	constexpr PieceType capturedPiece() const noexcept
	{
		return static_cast<PieceType>((_move >> CAPTURED_SHIFT) & CAPTURED_MASK);
	}

	constexpr void setCapturedPiece(const PieceType type) noexcept
	{
		constexpr u32 ShiftedMask = CAPTURED_MASK << CAPTURED_SHIFT;
		_move = (_move & ~ShiftedMask) | ((type << CAPTURED_SHIFT) & ShiftedMask);
	}

	constexpr PieceType promotedPiece() const noexcept
	{
		return static_cast<PieceType>((_move >> PROMOTED_SHIFT) & PROMOTED_MASK);
	}

	constexpr void setPromotedPiece(const PieceType type) noexcept
	{
		constexpr u32 ShiftedMask = PROMOTED_MASK << PROMOTED_SHIFT;
		_move = (_move & ~ShiftedMask) | ((type << PROMOTED_SHIFT) & ShiftedMask);
	}

	constexpr Square from() const noexcept
	{
		return toSquare((_move >> FROM_SHIFT) & FROM_MASK);
	}

	constexpr Square to() const noexcept
	{
		return toSquare((_move >> TO_SHIFT) & TO_MASK);
	}

	constexpr Flags flags() const noexcept
	{
		return Flags(u8(_move >> FLAGS_SHIFT) & FLAGS_MASK);
	}

	constexpr void setFlags(const u8 flags) noexcept
	{
		constexpr u32 ShiftedMask = FLAGS_MASK << FLAGS_SHIFT;
		_move = (_move & ~ShiftedMask) | ((flags << FLAGS_SHIFT) & ShiftedMask);
	}

	constexpr bool isTactical() const noexcept
	{
		const auto f = flags();
		return f.capture() | f.promotion();
	}

	constexpr u16 getFromToBits() const noexcept
	{
		constexpr u16 Mask = (FROM_MASK | (TO_MASK << TO_SHIFT));
		return _move & Mask;
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

		const Square fromSq = from();
		const Square toSq = to();

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

		str += 'a' + i8(fileOf(fromSq));
		str += '1' + i8(rankOf(fromSq));

		str += 'a' + i8(fileOf(toSq));
		str += '1' + i8(rankOf(toSq));

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
	 * Bits 0 to 5 (6 bits) store the 'from' square
	 * Bits 6 to 11 (6 bits) store the 'to' square
	 * Bits 12 to 14 (3 bits) store the 'moved' piece type
	 * Bits 15 to 17 (3 bits) store the 'captured' piece type
	 * Bits 18 to 20 (3 bits) store the 'promoted' piece type
	 * Bits 21 to 27 (7 bits) store the 'flags'
	 */
	u32 _move{};
	i32 _score{};

	static constexpr u32 FROM_SHIFT = 0;
	static constexpr u32 TO_SHIFT = 6u;
	static constexpr u32 MOVED_SHIFT = 12u;
	static constexpr u32 CAPTURED_SHIFT = 16u;
	static constexpr u32 PROMOTED_SHIFT = 20u;
	static constexpr u32 FLAGS_SHIFT = 23u;

	static constexpr u32 FROM_MASK = 0b11'1111;
	static constexpr u32 TO_MASK = 0b11'1111;
	static constexpr u32 MOVED_MASK = 0b111;
	static constexpr u32 CAPTURED_MASK = 0b111;
	static constexpr u32 PROMOTED_MASK = 0b111;
	static constexpr u32 FLAGS_MASK = 0b1111'1111;
};
