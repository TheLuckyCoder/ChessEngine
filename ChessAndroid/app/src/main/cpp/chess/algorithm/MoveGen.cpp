#include "MoveGen.h"

namespace
{
	template <Color Us>
	Move *generatePawnMoves(const Board &board, Move *moveList, const Bitboard targets)
	{
		using namespace Bits;

		constexpr Piece Piece{ PAWN, Us };
		constexpr Color Them = ~Us;
		constexpr Dir Forward = Us == WHITE ? NORTH : SOUTH;
		constexpr Bitboard StartRank{ Us == WHITE ? RANK_2 : RANK_7 };
		constexpr Bitboard LastRank{ Us == WHITE ? RANK_7 : RANK_2 };

		const auto addQuietMove = [&](const u8 from, const u8 to, const Bitboard pos)
		{
			Move move{ from, to, PAWN };

			if (LastRank & pos)
			{
				move.setFlags(Move::Flags::PROMOTION);
				for (u8 promotionType = KNIGHT; promotionType <= QUEEN; ++promotionType)
				{
					move.setPromotedPiece(PieceType(promotionType));
					*moveList++ = move;
				}
			} else
				*moveList++ = move;
		};

		const auto addCaptureMove = [&](const Square from, const Square to, const Bitboard pos)
		{
			Move move{ from, to, PAWN, Move::Flags::CAPTURE };
			move.setCapturedPiece(board.getPiece(to).type());

			if (LastRank & pos)
			{
				move.setFlags(Move::Flags::CAPTURE | Move::Flags::PROMOTION);
				for (u8 promotionType = KNIGHT; promotionType <= QUEEN; ++promotionType)
				{
					move.setPromotedPiece(PieceType(promotionType));
					*moveList++ = move;
				}
			} else
				*moveList++ = move;
		};

		for (u8 pieceNumber{}; pieceNumber < board.pieceCount[Piece]; ++pieceNumber)
		{
			const Square from = toSquare(board.pieceList[Piece][pieceNumber]);
			const auto bb = Bitboard::fromSquare(from);
			auto attacks = Attacks::pawnAttacks<Us>(bb);

			if (board.enPassantSq < SQ_NONE)
			{
				constexpr Dir EnPassantDirection = Us == WHITE ? SOUTH : NORTH;
				const auto enPassantCapture = Bitboard::fromSquare(board.enPassantSq);
				const auto capturedPawn = enPassantCapture.shift<EnPassantDirection>();

				if (board.getType(PAWN, Them) & capturedPawn && (attacks & enPassantCapture))
				{
					attacks &= ~enPassantCapture;
					*moveList++ = { from, board.enPassantSq, PAWN, Move::Flags::EN_PASSANT };
				}
			}

			attacks &= targets & board.occupied;

			while (attacks)
				addCaptureMove(from, attacks.popLsb(), bb);

			const Bitboard moveBB = bb.shift<Forward>();

			if (!(board.occupied & moveBB))
			{
				addQuietMove(from, moveBB.bitScanForward(), bb);

				if (StartRank & bb)
				{
					const auto doublePushBb = moveBB.shift<Forward>();

					if (!(board.occupied & doublePushBb))
					{
						const u8 targetSq = doublePushBb.bitScanForward();
						*moveList++ = { from, targetSq, PAWN,Move::Flags::DOUBLE_PAWN_PUSH };
					}
				}
			}
		}

		return moveList;
	}

	template <Color Us, PieceType P>
	Move *generatePieceMoves(const Board &board, Move *moveList, const Bitboard targets)
	{
		static_assert(P != KING && P != PAWN);
		constexpr Color Them = ~Us;
		constexpr Piece Piece{ P, Us };

		for (u8 pieceNumber{}; pieceNumber < board.pieceCount[Piece]; ++pieceNumber)
		{
			const Square from = toSquare(board.pieceList[Piece][pieceNumber]);

			Bitboard attacks;

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
				const Square to = attacks.popLsb();
				const auto bb = Bitboard::fromSquare(to);

				Move move{ from, to, P };
				if (bb & board.allPieces[Them])
				{
					move.setFlags(Move::Flags::CAPTURE);
					move.setCapturedPiece(board.getPiece(to).type());
				}

				*moveList++ = move;
			}
		}

		return moveList;
	}

	template <Color Us>
	Move *generateKingMoves(const Board &board, Move *moveList, const Bitboard targets)
	{
		constexpr Color Them = ~Us;
		constexpr Piece KingPiece{ KING, Us };

		const Square kingSq = board.getKingSq<Us>();
		assert(kingSq < SQUARE_NB);

		Bitboard moves = Attacks::kingAttacks(kingSq) & targets;
		moves &= ~Attacks::pawnAttacks<Them>(board.getType(PAWN, Them));

		const auto attackers = board.kingAttackers;
		if (attackers)
		{
			Bitboard bishopAttackers =
				attackers & (board.getType(BISHOP, Them) | board.getType(QUEEN, Them));
			Bitboard rookAttackers =
				attackers & (board.getType(ROOK, Them) | board.getType(QUEEN, Them));

			while (bishopAttackers)
				moves &= ~Bitboard::fromRayBetween(bishopAttackers.popLsb(), kingSq);

			while (rookAttackers)
				moves &= ~Bitboard::fromRayBetween(rookAttackers.popLsb(), kingSq);
		}

		while (moves)
		{
			const Square to = moves.popLsb();
			const auto bb = Bitboard::fromSquare(to);

			Move move{ kingSq, to, KING };
			if (bb & board.allPieces[Them])
			{
				move.setFlags(Move::Flags::CAPTURE);
				move.setCapturedPiece(board.getPiece(to).type());
			}
			*moveList++ = move;
		}

		if (board.isSideInCheck() || !board.canCastle<Us>())
			return moveList;

		const auto addCastleMove = [&, kingSq](const Square kingTo, const Square rookSq,
											   const Square rookTo, const u8 castleSide)
		{
			auto mask = Bitboard::fromRayBetween(kingSq, kingTo) | Bitboard::fromSquare(kingTo);
			mask |= Bitboard::fromRayBetween(rookSq, rookTo) | Bitboard::fromSquare(rookTo);
			mask &= ~(Bitboard::fromSquare(kingSq) | Bitboard::fromSquare(rookSq));

			// There can't be any pieces in between the rook and king
			if (board.occupied & mask)
				return;

			// The King can't pass through a checked square
			mask = Bitboard::fromRayBetween(kingSq, kingTo);
			while (mask)
			{
				if (board.isAttackedByAny(Them, mask.popLsb()))
					return;
			}

			*moveList++ = { kingSq, kingTo, KING, castleSide };
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

		return moveList;
	}

	template <Color Us>
	Move *generateAllMoves(const Board &board, Move *moveList)
	{
		using namespace Bits;
		constexpr Color Them = ~Us;

		const auto kingTargets = ~board.allPieces[Us];
		if (board.kingAttackers.several())
			return generateKingMoves<Us>(board, moveList, kingTargets);

		Bitboard targets;
		const auto kingAttackers = board.kingAttackers;
		// When checked we must either capture the attacker
		// or block it if is a slider piece
		if (kingAttackers)
		{
			targets |= kingAttackers;

			Bitboard bishopAttackers =
				kingAttackers & (board.getType(BISHOP, Them) | board.getType(QUEEN, Them));
			Bitboard rookAttackers =
				kingAttackers & (board.getType(ROOK, Them) | board.getType(QUEEN, Them));

			while (bishopAttackers)
				targets |= Attacks::bishopXRayAttacks(bishopAttackers.popLsb());

			while (rookAttackers)
				targets |= Attacks::rookXRayAttacks(rookAttackers.popLsb());
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
