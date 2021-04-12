#include "MoveGen.h"

namespace
{
	enum class GenType
	{
		ALL,
		EVASIONS
	};

	template <Color Us>
	void generatePawnMoves(const Board &board, MoveList &moveList, const Bitboard targets, const GenType type)
	{
		constexpr Color Them{ ~Us };
		constexpr Dir Forward{ Us == WHITE ? NORTH : SOUTH };
		constexpr Dir Backward{ Us == WHITE ? SOUTH : NORTH };
		constexpr Bitboard ThirdRank{ Us == WHITE ? RANK_3 : RANK_6 };
		constexpr Bitboard LastRank{ Us == WHITE ? RANK_7 : RANK_2 };

		const Bitboard pawns = board.getPieces(PAWN, Us);
		const Bitboard pawnsOnLastRank = pawns & LastRank;
		const Bitboard pawnsNotOnLastRank = pawns & ~pawnsOnLastRank;

		const Bitboard enemies = type == GenType::EVASIONS ? board.getKingAttackers() : board.getPieces(Them);
		const Bitboard emptySquares = ~board.getPieces();

		// Promotions
		if (pawnsOnLastRank.notEmpty())
		{
			auto forward = pawnsOnLastRank.shift<Forward>() & emptySquares;
			auto left = pawnsOnLastRank.shift<Forward, WEST>() & enemies;
			auto right = pawnsOnLastRank.shift<Forward, EAST>() & enemies;

			if (type == GenType::EVASIONS)
				forward &= targets;

			const auto makePromotions = [&](Square from, Square to)
			{
				const auto captured = board.getPiece(to).type();
				u8 flags = Move::Flags::PROMOTION;
				if (captured != PIECE_TYPE_NB)
					flags |= Move::Flags::CAPTURE;

				Move move(from, to, PAWN, flags);
				for (u8 promotionType = QUEEN; promotionType >= KNIGHT; --promotionType)
				{
					move.setPromotedPiece(PieceType(promotionType));
					move.setCapturedPiece(captured);
					moveList.emplace_back(move);
				}
			};

			while (forward.notEmpty())
			{
				const Square to = forward.popLsb();
				makePromotions(shift<Backward>(to), to);
			}

			while (left.notEmpty())
			{
				const Square to = left.popLsb();
				makePromotions(shift<EAST>(shift<Backward>(to)), to);
			}

			while (right.notEmpty())
			{
				const Square to = right.popLsb();
				makePromotions(shift<WEST>(shift<Backward>(to)), to);
			}
		}

		// EnPassant
		const Square enPassantSq = board.getEnPassant();
		if (enPassantSq != SQ_NONE)
		{
			const auto enPassant = Bitboard::fromSquare(enPassantSq);

			if (type != GenType::EVASIONS || (targets & enPassant.shift<Forward>()).empty())
			{
				auto pawnsThatCapture = Attacks::pawnAttacks<Them>(enPassant) & pawnsNotOnLastRank;

				while (pawnsThatCapture.notEmpty())
				{
					const Square from = pawnsThatCapture.popLsb();
					moveList.emplace_back(from, enPassantSq, PAWN, Move::Flags::EN_PASSANT);
				}
			}
		}

		// Captures
		{
			auto left = pawnsNotOnLastRank.shift<Forward, WEST>() & enemies;
			auto right = pawnsNotOnLastRank.shift<Forward, EAST>() & enemies;

			if (type == GenType::EVASIONS)
			{
				left &= targets;
				right &= targets;
			}

			const auto makeCapture = [&](Square from, Square to)
			{
				Move move(from, to, PAWN, Move::Flags::CAPTURE);
				move.setCapturedPiece(board.getPiece(to).type());
				moveList.emplace_back(move);
			};

			while (left.notEmpty())
			{
				const Square to = left.popLsb();
				makeCapture(shift<EAST>(shift<Backward>(to)), to);
			}

			while (right.notEmpty())
			{
				const Square to = right.popLsb();
				makeCapture(shift<WEST>(shift<Backward>(to)), to);
			}
		}

		// Pushes and Double Pushes
		auto pushes = pawnsNotOnLastRank.shift<Forward>() & emptySquares;
		auto doublePushes = (pushes & ThirdRank).template shift<Forward>() & emptySquares;

		if (type == GenType::EVASIONS)
		{
			pushes &= targets;
			doublePushes &= targets;
		}

		while (pushes.notEmpty())
		{
			const Square to = pushes.popLsb();
			moveList.emplace_back(shift<Backward>(to), to, PAWN);
		}

		while (doublePushes.notEmpty())
		{
			const Square to = doublePushes.popLsb();
			moveList.emplace_back(shift<Backward>(shift<Backward>(to)), to, PAWN, Move::Flags::DOUBLE_PAWN_PUSH);
		}
	}

	template <Color Us, PieceType P>
	void generatePieceMoves(const Board &board, MoveList &moveList, const Bitboard targets)
	{
		static_assert(P > PAWN && P < KING, "Unsupported Piece Type");

		Bitboard pieces = board.getPieces(P, Us);

		while (pieces.notEmpty())
		{
			const Square from = pieces.popLsb();

			Bitboard attacks{};

			if constexpr (P == KNIGHT)
				attacks = Attacks::knightAttacks(from);
			else if constexpr (P == BISHOP)
				attacks = Attacks::bishopAttacks(from, board.getPieces());
			else if constexpr (P == ROOK)
				attacks = Attacks::rookAttacks(from, board.getPieces());
			else if constexpr (P == QUEEN)
				attacks = Attacks::queenAttacks(from, board.getPieces());

			attacks &= targets;

			while (attacks.notEmpty())
			{
				const Square to = attacks.popLsb();

				Move move{ from, to, P };
				if (const PieceType capturedPiece = board.getPiece(to).type();
					capturedPiece != NO_PIECE_TYPE)
				{
					move.setFlags(Move::Flags::CAPTURE);
					move.setCapturedPiece(capturedPiece);
				}
				moveList.emplace_back(move);
			}
		}
	}

	template <Color Us>
	void generateKingMoves(const Board &board, MoveList &moveList, const Bitboard targets)
	{
		assert(board.getKingAttackers().empty());
		constexpr Color Them{ ~Us };

		const Square kingSq = board.getKingSq(Us);

		Bitboard moves = Attacks::kingAttacks(kingSq) & targets;

		while (moves.notEmpty())
		{
			const Square to = moves.popLsb();

			Move move{ kingSq, to, KING };
			if (const PieceType capturedPiece = board.getPiece(to).type();
				capturedPiece != NO_PIECE_TYPE)
			{
				move.setFlags(Move::Flags::CAPTURE);
				move.setCapturedPiece(capturedPiece);
			}
			moveList.emplace_back(move);
		}

		if (!board.canCastle<Us>())
			return;

		const auto addCastleMove = [&, kingSq](const Square kingTo, const Square rookSq,
											   const Square rookTo, const u8 castleSide)
		{
			auto mask = Bitboard::fromLineBetween(kingSq, kingTo) | Bitboard::fromSquare(kingTo);
			mask |= Bitboard::fromLineBetween(rookSq, rookTo) | Bitboard::fromSquare(rookTo);
			mask &= ~(Bitboard::fromSquare(kingSq) | Bitboard::fromSquare(rookSq));

			// There can't be any pieces in between the rook and king
			if ((board.getPieces() & mask).notEmpty())
				return;

			// The King can't pass through a checked square
			mask = Bitboard::fromLineBetween(kingSq, kingTo);
			while (mask.notEmpty())
			{
				// TODO Move this in Board::isMoveLegal()
				if (board.isAttackedByAny(Them, mask.popLsb(), board.getPieces()).notEmpty())
					return;
			}

			moveList.emplace_back(kingSq, kingTo, KING, castleSide);
		};

		if (board.canCastleKs<Us>())
		{
			constexpr Square KingTo = Us ? SQ_G1 : SQ_G8;
			constexpr Square RookSq = Us ? SQ_H1 : SQ_H8;
			constexpr Square RookTo = Us ? SQ_F1 : SQ_F8;
			addCastleMove(KingTo, RookSq, RookTo, Move::Flags::KSIDE_CASTLE);
		}

		if (board.canCastleQs<Us>())
		{
			constexpr Square KingTo = Us ? SQ_C1 : SQ_C8;
			constexpr Square RookSq = Us ? SQ_A1 : SQ_A8;
			constexpr Square RookTo = Us ? SQ_D1 : SQ_D8;
			addCastleMove(KingTo, RookSq, RookTo, Move::Flags::QSIDE_CASTLE);
		}
	}

	template <Color Us>
	void generateAllMoves(const Board &board, MoveList &moveList)
	{
		auto type = GenType::ALL;
		auto targets = ~board.getPieces(Us); // Everywhere but our pieces
		const auto kingAttackers = board.getKingAttackers();

		// When checked we must either capture the attacker
		// or block it if is a slider piece
		if (kingAttackers.notEmpty())
		{
			const Square kingSq = board.getKingSq(Us);
			auto sliders = kingAttackers & ~(board.getPieces(PAWN) | board.getPieces(KNIGHT));
			Bitboard sliderAttacks;

			while (sliders.notEmpty())
				sliderAttacks |= Bitboard::fromLine(kingSq, sliders.popLsb()) & ~kingAttackers;

			// Evasions for king, capture and non capture moves
			auto moves = Attacks::kingAttacks(kingSq) & ~board.getPieces(Us) & ~sliderAttacks;
			while (moves.notEmpty())
			{
				const Square to = moves.popLsb();

				Move move{ kingSq, to, KING };
				if (const PieceType capturedPiece = board.getPiece(to).type();
					capturedPiece != NO_PIECE_TYPE)
				{
					move.setFlags(Move::Flags::CAPTURE);
					move.setCapturedPiece(capturedPiece);
				}
				moveList.emplace_back(move);
			}

			// We can't to anything else if there are two checkers
			if (kingAttackers.several())
				return;

			const Square checkSq = kingAttackers.bitScanForward();
			targets &= Bitboard::fromLineBetween(kingSq, checkSq) | kingAttackers;
			type = GenType::EVASIONS;
		}

		generatePawnMoves<Us>(board, moveList, targets, type);
		generatePieceMoves<Us, KNIGHT>(board, moveList, targets);
		generatePieceMoves<Us, BISHOP>(board, moveList, targets);
		generatePieceMoves<Us, ROOK>(board, moveList, targets);
		generatePieceMoves<Us, QUEEN>(board, moveList, targets);

		if (kingAttackers.empty())
			generateKingMoves<Us>(board, moveList, targets);
	}
}

void MoveList::generateMoves() noexcept
{
	if (_board.colorToMove == WHITE)
		generateAllMoves<WHITE>(_board, *this);
	else
		generateAllMoves<BLACK>(_board, *this);
}
