#pragma once

#include <string>

#include "Defs.h"
#include "Bitfield.h"

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
			KSIDE_CASTLE = 1 << 2, // The move is a king side castle
			QSIDE_CASTLE = 1 << 3, // The move is a queen side castle
			DOUBLE_PAWN_PUSH = 1 << 4, // The move is a double pawn push
			EN_PASSANT = 1 << 5, // The move is an en passant capture (Do not set the CAPTURE flag too)
			PV_MOVE = 1 << 6, // The move is a PV Move
		};

	public:
		explicit constexpr Flags(const u8 flags) noexcept: _flags(flags) {}

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

	constexpr Move() = default;

	explicit constexpr Move(const u32 move, const i32 score = 0) noexcept
		: _move(move), _score(score)
	{
	}

	constexpr Move(const u8 from, const u8 to, const PieceType piece) noexcept
	{
		_move.set<0>(from);
		_move.set<1>(to);
		_move.setAs<2>(piece);
	}

	constexpr Move(const u8 from, const u8 to, const PieceType piece, const u8 flags) noexcept
		: Move(from, to, piece)
	{
		setFlags(flags);
	}

	constexpr bool empty() const noexcept { return !static_cast<bool>(_move.value()); }

	constexpr u32 getContents() const noexcept { return _move.value(); }

	constexpr i32 getScore() const noexcept { return _score; }

	constexpr void setScore(const int score) noexcept
	{
		_score = score;
	}

	constexpr Square from() const noexcept
	{
		return toSquare(_move.get<0>());
	}

	constexpr Square to() const noexcept
	{
		return toSquare(_move.get<1>());
	}

	constexpr PieceType piece() const noexcept
	{
		return _move.getAs<2, PieceType>();
	}

	constexpr PieceType capturedPiece() const noexcept
	{
		return _move.getAs<3, PieceType>();
	}

	constexpr void setCapturedPiece(const PieceType type) noexcept
	{
		_move.setAs<3>(type);
	}

	constexpr PieceType promotedPiece() const noexcept
	{
		return _move.getAs<4, PieceType>();
	}

	constexpr void setPromotedPiece(const PieceType type) noexcept
	{
		_move.setAs<4>(type);
	}

	constexpr Flags flags() const noexcept
	{
		return _move.getAs<5, Flags>();
	}

	constexpr void setFlags(const u8 flags) noexcept
	{
		_move.set<5>(flags);
	}

	constexpr bool isTactical() const noexcept
	{
		const auto f = flags();
		return f.capture() | f.promotion();
	}

	constexpr u16 getFromToBits() const noexcept
	{
		constexpr u16 Mask = (0x3Fu | (0x3Fu << 6u));
		return _move.value() & Mask;
	}

	constexpr bool operator==(const Move &rhs) const noexcept
	{
		return _move.value() == rhs._move.value();
	}

	constexpr bool operator!=(const Move &rhs) const noexcept
	{
		return _move.value() != rhs._move.value();
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
	 * Bits 0 to 5 (6 bits) - 'from' square
	 * Bits 6 to 11 (6 bits) - 'to' square
	 * Bits 12 to 14 (3 bits) - 'moved' piece type
	 * Bits 15 to 17 (3 bits) - 'captured' piece type
	 * Bits 18 to 20 (3 bits) - 'promoted' piece type
	 * Bits 21 to 27 (7 bits) - 'flags'
	 */
	Bitfield<u32, 6, 6, 3, 3, 3, 7> _move{};
	i32 _score{};
};
