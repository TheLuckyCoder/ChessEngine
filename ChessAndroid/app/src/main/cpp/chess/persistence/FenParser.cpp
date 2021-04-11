#include "FenParser.h"

#include "../algorithm/Evaluation.h"
#include "../Zobrist.h"

bool FenParser::parseFen(Board &board, const std::string &fen)
{
	std::istringstream stream(fen);

	if (!stream) return false;

	// Clean board
	board = {};

	parsePieces(board, stream);

	// Next to move
	std::string side;
	stream >> side;
	Color &colorToMove = board.colorToMove;
	colorToMove = side == "w" ? WHITE : BLACK;

	// Castling availability
	std::string castling = "-";
	stream >> castling;
	auto &castlingRights = board.state.castlingRights;
	castlingRights = CastlingRights::CASTLE_NONE;
	for (char currChar : castling) {
		switch (currChar) {
			case 'K': castlingRights |= CastlingRights::CASTLE_WHITE_KING;
				break;
			case 'Q': castlingRights |= CastlingRights::CASTLE_WHITE_QUEEN;
				break;
			case 'k': castlingRights |= CastlingRights::CASTLE_BLACK_KING;
				break;
			case 'q': castlingRights |= CastlingRights::CASTLE_BLACK_QUEEN;
				break;
			default:
				break;
		}
	}

	std::string ep = "-";
	stream >> ep;
	auto &enPassantSq = board.state.enPassantSq;
	enPassantSq = SQUARE_NB;
	if (ep != "-")
	{
		if (ep.length() != 2)
			return false;
		if (!(ep[0] >= 'a' && ep[0] <= 'h'))
			return false;

		const Square sq = ::toSquare(int(ep[0] - 'a'), int(ep[1] - '1'));
		enPassantSq = sq < SQUARE_NB ? sq : SQUARE_NB;
	}

	// HalfMove Clock
	int halfMove{};
	stream >> halfMove;
	board.state.fiftyMoveRule = static_cast<u8>(halfMove);

	board.state.zKey = Zobrist::compute(board);
	board.updatePieceList();
	board.updateNonPieceBitboards();

	board.state.kingAttackers = board.generateAllAttackers(board.getKingSq(colorToMove)) & board.getPieces(~colorToMove);
	board.computeCheckInfo();
	return true;
}

std::string FenParser::exportToFen(const Board &board)
{
	std::ostringstream out;

	for (int rank = 7; rank >= 0; --rank)
	{
		for (int file = 0; file <= 7; ++file)
		{
			int emptyCount = 0;
			for (; file <= 7 && board.data[toSquare(file, rank)] == EmptyPiece; ++file)
				++emptyCount;

			if (emptyCount)
				out << emptyCount;

			if (file <= 7) {
				const Piece piece = board.data[toSquare(file, rank)];
				const PieceType type = piece.type();
				char pChar = 'K';

				if (type == PAWN)
					pChar = 'P';
				else if (type == KNIGHT)
					pChar = 'N';
				else if (type == BISHOP)
					pChar = 'B';
				else if (type == ROOK)
					pChar = 'R';
				else if (type == QUEEN)
					pChar = 'Q';

				if (piece.color() == BLACK)
					pChar += 32;

				out << pChar;
			}
		}

		if (rank > 0)
			out << '/';
	}

	out << (board.colorToMove == WHITE ? " w " : " b ");

	if (board.canCastleKs<WHITE>()) out << 'K';

	if (board.canCastleQs<WHITE>()) out << 'Q';

	if (board.canCastleKs<BLACK>()) out << 'k';

	if (board.canCastleQs<BLACK>()) out << 'q';

	if (!board.canCastle<WHITE>() && !board.canCastle<BLACK>()) out << '-';

	const Square enPassantSq = board.getEnPassant();
	if (enPassantSq != SQ_NONE)
		out << ' ' << char('a' + int(fileOf(enPassantSq))) << int(rankOf(enPassantSq)) << ' ';
	else
		out << " - ";

	out << short(board.state.fiftyMoveRule / 2);

	return out.str();
}

void FenParser::parsePieces(Board &board, std::istringstream &stream)
{
	std::string token;
	token.reserve(32);

	u64 boardPos = 56ull; // Fen string starts at a8 = index 56
	stream >> token;
	for (auto currChar : token)
	{
		switch (currChar)
		{
			case 'p':
				board.data[boardPos++] = { PAWN, BLACK };
				break;
			case 'r':
				board.data[boardPos++] = { ROOK, BLACK };
				break;
			case 'n':
				board.data[boardPos++] = { KNIGHT, BLACK };
				break;
			case 'b':
				board.data[boardPos++] = { BISHOP, BLACK };
				break;
			case 'q':
				board.data[boardPos++] = { QUEEN, BLACK };
				break;
			case 'k':
				board.data[boardPos++] = { KING, BLACK };
				break;
			case 'P':
				board.data[boardPos++] = { PAWN, WHITE };
				break;
			case 'R':
				board.data[boardPos++] = { ROOK, WHITE };
				break;
			case 'N':
				board.data[boardPos++] = { KNIGHT, WHITE };
				break;
			case 'B':
				board.data[boardPos++] = { BISHOP, WHITE };
				break;
			case 'Q':
				board.data[boardPos++] = { QUEEN, WHITE };
				break;
			case 'K':
				board.data[boardPos++] = { KING, WHITE };
				break;
			case '/': boardPos -= 16u; // Go down one rank
				break;
			default:
				boardPos += static_cast<u64>(currChar - '0');
		}
	}
}
