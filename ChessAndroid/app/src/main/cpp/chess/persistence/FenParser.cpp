#include "FenParser.h"

#include "../data/Board.h"
#include "../algorithm/Hash.h"

FenParser::FenParser(Board &board)
	: board(board)
{
}

void FenParser::parseFen(const std::string &fen)
{
	std::istringstream fenStream(fen);

	// Clean board
	board.data.fill({});
	board.pieces.fill({});
	board.occupied = 0ull;
	board.npm = 0;

	parsePieces(fenStream);

	// Next to move
	std::string token;
	fenStream >> token;
	board.colorToMove = token == "w" ? WHITE : BLACK;

	// Castling availability
	fenStream >> token;
	board.castlingRights = CastlingRights::CASTLE_NONE;
	for (auto &currChar : token) {
		switch (currChar) {
			case 'K': board.castlingRights |= CastlingRights::CASTLE_WHITE_KING;
				break;
			case 'Q': board.castlingRights |= CastlingRights::CASTLE_WHITE_QUEEN;
				break;
			case 'k': board.castlingRights |= CastlingRights::CASTLE_BLACK_KING;
				break;
			case 'q': board.castlingRights |= CastlingRights::CASTLE_BLACK_QUEEN;
				break;
			default:
				break;
		}
	}

	// TODO: En passant target square
	fenStream >> token;
	board.enPassantSq = 64u;
	//board.enPassant = token == "-" ? 0ull : 1 << x;


	// Halfmove Clock
	int halfMove;
	fenStream >> halfMove;
	board.halfMoveClock = static_cast<byte>(halfMove);

	board.updateNonPieceBitboards();
	board.zKey = Hash::compute(board);
}

std::string FenParser::exportToFen()
{
	return std::string();
}

void FenParser::parsePieces(std::istringstream &stream)
{
	std::string token;

	U64 boardPos = 56ull; // Fen string starts at a8 = index 56
	stream >> token;
	for (auto currChar : token)
	{
		switch (currChar)
		{
			case 'p':
				board.data[boardPos++] = Piece(PAWN, BLACK);
				break;
			case 'r':
				board.data[boardPos++] = Piece(ROOK, BLACK);
				break;
			case 'n':
				board.data[boardPos++] = Piece(KNIGHT, BLACK);
				break;
			case 'b':
				board.data[boardPos++] = Piece(BISHOP, BLACK);
				break;
			case 'q':
				board.data[boardPos++] = Piece(QUEEN, BLACK);
				break;
			case 'k':
				board.data[boardPos++] = Piece(KING, BLACK);
				break;
			case 'P':
				board.data[boardPos++] = Piece(PAWN, WHITE);
				break;
			case 'R':
				board.data[boardPos++] = Piece(ROOK, WHITE);
				break;
			case 'N':
				board.data[boardPos++] = Piece(KNIGHT, WHITE);
				break;
			case 'B':
				board.data[boardPos++] = Piece(BISHOP, WHITE);
				break;
			case 'Q':
				board.data[boardPos++] = Piece(QUEEN, WHITE);
				break;
			case 'K':
				board.data[boardPos++] = Piece(KING, WHITE);
				break;
			case '/': boardPos -= 16u; // Go down one rank
				break;
			default:
				boardPos += static_cast<U64>(currChar - '0');
		}
	}

	for (byte i = 0u; i < 64u; ++i)
	{
		const Piece &piece = board.data[i];
		const U64 bb = Bitboard::shiftedBoards[i];

		board.getType(piece) |= bb;

		if (piece.type() != PAWN)
			board.npm += Evaluation::getPieceValue(piece.type());
	}
}
