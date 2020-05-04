#include "FenParser.h"

#include "../algorithm/Evaluation.h"
#include "../data/Bits.h"
#include "../data/Board.h"
#include "../algorithm/Hash.h"

FenParser::FenParser(Board &board)
	: _board(board)
{
}

void FenParser::parseFen(const std::string &fen)
{
	std::istringstream fenStream(fen);

	// Clean board
	_board.data.fill({});
	_board.pieces.fill({});
	_board.occupied = 0ull;
	_board.npm = 0;

	parsePieces(fenStream);

	// Next to move
	std::string token;
	fenStream >> token;
	_board.colorToMove = token == "w" ? WHITE : BLACK;

	// Castling availability
	fenStream >> token;
	_board.castlingRights = CastlingRights::CASTLE_NONE;
	for (char currChar : token) {
		switch (currChar) {
			case 'K': _board.castlingRights |= CastlingRights::CASTLE_WHITE_KING;
				break;
			case 'Q': _board.castlingRights |= CastlingRights::CASTLE_WHITE_QUEEN;
				break;
			case 'k': _board.castlingRights |= CastlingRights::CASTLE_BLACK_KING;
				break;
			case 'q': _board.castlingRights |= CastlingRights::CASTLE_BLACK_QUEEN;
				break;
			default:
				break;
		}
	}

	// TODO: En passant target square
	fenStream >> token;
	_board.enPassantSq = 64u;
	//_board.enPassant = token == "-" ? 0ull : 1 << x;

	// Halfmove Clock
	int halfMove;
	fenStream >> halfMove;
	_board.fiftyMoveRule = static_cast<byte>(halfMove);

	_board.updatePieceList();
	_board.updateNonPieceBitboards();
	_board.zKey = Hash::compute(_board);
}

std::string FenParser::exportToFen()
{
	return std::string();
}

void FenParser::parsePieces(std::istringstream &stream) const
{
	std::string token;

	U64 boardPos = 56ull; // Fen string starts at a8 = index 56
	stream >> token;
	for (auto currChar : token)
	{
		switch (currChar)
		{
			case 'p':
				_board.data[boardPos++] = Piece(PAWN, BLACK);
				break;
			case 'r':
				_board.data[boardPos++] = Piece(ROOK, BLACK);
				break;
			case 'n':
				_board.data[boardPos++] = Piece(KNIGHT, BLACK);
				break;
			case 'b':
				_board.data[boardPos++] = Piece(BISHOP, BLACK);
				break;
			case 'q':
				_board.data[boardPos++] = Piece(QUEEN, BLACK);
				break;
			case 'k':
				_board.data[boardPos++] = Piece(KING, BLACK);
				break;
			case 'P':
				_board.data[boardPos++] = Piece(PAWN, WHITE);
				break;
			case 'R':
				_board.data[boardPos++] = Piece(ROOK, WHITE);
				break;
			case 'N':
				_board.data[boardPos++] = Piece(KNIGHT, WHITE);
				break;
			case 'B':
				_board.data[boardPos++] = Piece(BISHOP, WHITE);
				break;
			case 'Q':
				_board.data[boardPos++] = Piece(QUEEN, WHITE);
				break;
			case 'K':
				_board.data[boardPos++] = Piece(KING, WHITE);
				break;
			case '/': boardPos -= 16u; // Go down one rank
				break;
			default:
				boardPos += static_cast<U64>(currChar - '0');
		}
	}

	for (byte square = 0u; square < SQUARE_NB; ++square)
	{
		const Piece piece = _board.data[square];
		const U64 bb = Bits::shiftedBoards[square];

		_board.getType(piece) |= bb;

		if (piece.type() != PAWN)
			_board.npm += Evaluation::getPieceValue(piece.type());
	}
}
