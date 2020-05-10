#include "MoveGen.h"

#include "Attacks.h"

using namespace Bits;

namespace
{
	template <Color Us>
	Move *generatePawnMoves(const Board &board, Move *moveList, const U64 targets)
	{
		using namespace Bits;

		constexpr Piece Piece{ PAWN, Us };
		constexpr Color Them = ~Us;
		constexpr Dir Forward = Us == WHITE ? NORTH : SOUTH;
		constexpr U64 StartRank = Us == WHITE ? RANK_2 : RANK_7;
		constexpr U64 LastRank = Us == WHITE ? RANK_7 : RANK_2;

		const auto addQuietMove = [&](const byte from, const byte to, const U64 pos)
		{
			Move move{ from, to, PAWN };

			if (LastRank & pos)
			{
				move.setFlags(Move::PROMOTION);
				for (byte promotionType = KNIGHT; promotionType <= QUEEN; ++promotionType)
				{
					move.setPromotedPiece(PieceType(promotionType));
					*moveList++ = move;
				}
			} else
				*moveList++ = move;
		};

		const auto addCaptureMove = [&](const byte from, const byte to, const U64 pos)
		{
			Move move{ from, to, PAWN, Move::CAPTURE };
			move.setCapturedPiece(board.getPiece(to).type());

			if (LastRank & pos)
			{
				move.setFlags(Move::CAPTURE | Move::PROMOTION);
				for (byte promotionType = KNIGHT; promotionType <= QUEEN; ++promotionType)
				{
					move.setPromotedPiece(PieceType(promotionType));
					*moveList++ = move;
				}
			} else
				*moveList++ = move;
		};

		for (byte pieceNumber{}; pieceNumber < board.pieceCount[Piece]; ++pieceNumber)
		{
			const byte from = board.pieceList[Piece][pieceNumber];
			const U64 bb = getSquare64(from);
			U64 attacks = Attacks::pawnAttacks<Us>(bb);

			if (board.enPassantSq < SQ_NONE)
			{
				constexpr Dir EnPassantDirection = Us == WHITE ? SOUTH : NORTH;
				const U64 enPassantCapture = getSquare64(board.enPassantSq);
				const U64 capturedPawn = shift<EnPassantDirection>(enPassantCapture);

				if (board.getType(PAWN, Them) & capturedPawn && (attacks & enPassantCapture))
				{
					attacks &= ~enPassantCapture;
					*moveList++ = { from, board.enPassantSq, PAWN, Move::EN_PASSANT };
				}
			}

			attacks &= targets & board.occupied;

			while (attacks)
			{
				const byte to = popLsb(attacks);
				addCaptureMove(from, to, bb);
			}

			const U64 moveBB = shift<Forward>(bb);

			if (!(board.occupied & moveBB))
			{
				addQuietMove(from, bitScanForward(moveBB), bb);

				if (StartRank & bb)
				{
					const U64 doublePushBb = shift<Forward>(moveBB);

					if (!(board.occupied & doublePushBb))
						*moveList++ = { from, bitScanForward(doublePushBb), PAWN,
										Move::DOUBLE_PAWN_PUSH };
				}
			}
		}

		return moveList;
	}

	template <Color Us, PieceType P>
	Move *generatePieceMoves(const Board &board, Move *moveList, const U64 targets)
	{
		static_assert(P != KING && P != PAWN);
		constexpr Color Them = ~Us;
		constexpr Piece Piece{ P, Us };

		for (byte pieceNumber{}; pieceNumber < board.pieceCount[Piece]; ++pieceNumber)
		{
			const byte from = board.pieceList[Piece][pieceNumber];

			U64 attacks{};

			if constexpr (P == KNIGHT)
				attacks = Attacks::knightAttacks(from);
			else if constexpr (P == BISHOP)
				attacks = Attacks::bishopAttacks(from, board.occupied);
			else if constexpr (P == ROOK)
				attacks = Attacks::rookAttacks(from, board.occupied);
			else if constexpr (P == QUEEN)
				attacks = Attacks::queenAttacks(from, board.occupied);

			attacks &= targets;

			while (attacks)
			{
				const byte to = popLsb(attacks);
				const U64 bb = getSquare64(to);

				Move move{ from, to, P };
				if (bb & board.allPieces[Them])
				{
					move.setFlags(Move::CAPTURE);
					move.setCapturedPiece(board.getPiece(to).type());
				}

				*moveList++ = move;
			}
		}

		return moveList;
	}

	template <Color Us>
	Move *generateKingMoves(const Board &board, Move *moveList, const U64 targets)
	{
		constexpr Color Them = ~Us;
		constexpr Piece KingPiece{ KING, Us };

		const byte kingSq = board.pieceList[KingPiece][0];
		assert(kingSq < SQUARE_NB);

		U64 moves = Attacks::kingAttacks(kingSq) & targets;
		moves &= ~Attacks::pawnAttacks<Them>(board.getType(PAWN, Them));

		const U64 attackers = board.kingAttackers;
		if (attackers)
		{
			U64 bishopAttackers =
				attackers & (board.getType(BISHOP, Them) | board.getType(QUEEN, Them));
			U64 rookAttackers =
				attackers & (board.getType(ROOK, Them) | board.getType(QUEEN, Them));

			while (bishopAttackers)
				moves &= ~Bits::getRayBetween(popLsb(bishopAttackers), kingSq);

			while (rookAttackers)
				moves &= ~Bits::getRayBetween(popLsb(rookAttackers), kingSq);
		}

		while (moves)
		{
			const byte to = popLsb(moves);
			const U64 bb = getSquare64(to);

			Move move{ kingSq, to, KING };
			if (bb & board.allPieces[Them])
			{
				move.setFlags(Move::CAPTURE);
				move.setCapturedPiece(board.getPiece(to).type());
			}
			*moveList++ = move;
		}

		if (board.isSideInCheck() || !board.canCastle<Us>())
			return moveList;

		const auto addCastleMove = [&, kingSq](const byte kingTo, const byte rookSq,
											   const byte rookTo, const Move::Flag castleSide)
		{
			U64 mask = getRayBetween(kingSq, kingTo) | getSquare64(kingTo);
			mask |= getRayBetween(rookSq, rookTo) | getSquare64(rookTo);
			mask &= ~(getSquare64(kingSq) | getSquare64(rookSq));

			// There can't be any pieces in between the rook and king
			if (board.occupied & mask)
				return;

			// The King can't pass through a checked square
			mask = getRayBetween(kingSq, kingTo);
			while (mask)
			{
				if (board.isAttackedByAny(Them, popLsb(mask)))
					return;
			}

			*moveList++ = { kingSq, kingTo, KING, static_cast<unsigned>(castleSide) };
		};

		if (board.canCastleKs<Us>())
		{
			constexpr Square KingTo = Us ? SQ_G1 : SQ_G8;
			constexpr Square RookSq = Us ? SQ_H1 : SQ_H8;
			constexpr Square RookTo = Us ? SQ_F1 : SQ_F8;
			addCastleMove(KingTo, RookSq, RookTo, Move::KSIDE_CASTLE);
		}

		if (board.canCastleQs<Us>())
		{
			constexpr Square KingTo = Us ? SQ_C1 : SQ_C8;
			constexpr Square RookSq = Us ? SQ_A1 : SQ_A8;
			constexpr Square RookTo = Us ? SQ_D1 : SQ_D8;
			addCastleMove(KingTo, RookSq, RookTo, Move::QSIDE_CASTLE);
		}

		return moveList;
	}

	template <Color Us>
	Move *generateAllMoves(const Board &board, Move *moveList)
	{
		using namespace Bits;
		constexpr Color Them = ~Us;

		const U64 kingTargets = ~board.allPieces[Us];
		if (several(board.kingAttackers))
			return generateKingMoves<Us>(board, moveList, kingTargets);

		U64 targets{};
		const U64 kingAttackers = board.kingAttackers;
		// When checked we must either capture the attacker
		// or block it if is a slider piece
		if (kingAttackers)
		{
			targets |= kingAttackers;

			U64 bishopAttackers =
				kingAttackers & (board.getType(BISHOP, Them) | board.getType(QUEEN, Them));
			U64 rookAttackers =
				kingAttackers & (board.getType(ROOK, Them) | board.getType(QUEEN, Them));

			while (bishopAttackers)
			{
				const byte sq = popLsb(bishopAttackers);
				targets |= Attacks::bishopXRayAttacks(sq);
			}

			while (rookAttackers)
			{
				const byte sq = popLsb(rookAttackers);
				targets |= Attacks::rookXRayAttacks(sq);
			}
		}

		// Everywhere but our pieces
		if (targets)
			targets &= kingTargets;
		else
			targets = kingTargets;

		moveList = generatePawnMoves<Us>(board, moveList, targets);
		moveList = generatePieceMoves<Us, KNIGHT>(board, moveList, targets);
		moveList = generatePieceMoves<Us, BISHOP>(board, moveList, targets);
		moveList = generatePieceMoves<Us, ROOK>(board, moveList, targets);
		moveList = generatePieceMoves<Us, QUEEN>(board, moveList, targets);
		moveList = generateKingMoves<Us>(board, moveList, kingTargets);

		return moveList;
	}
}

Move *generateMoves(const Board &board, Move *moveList) noexcept
{
	const Color color = board.colorToMove;
	return color == WHITE
		   ? generateAllMoves<WHITE>(board, moveList)
		   : generateAllMoves<BLACK>(board, moveList);
}
