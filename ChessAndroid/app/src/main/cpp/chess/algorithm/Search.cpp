#include "Search.h"

#include <algorithm>
#include <atomic>
#include <mutex>

#include "../Stats.h"
#include "../data/Board.h"

ThreadPool Search::s_ThreadPool(1);
TranspositionTable Search::s_SearchCache(1);
bool Search::s_QuiescenceSearchEnabled{};
short Search::s_BestMoveFound{};

RootMove Search::findBestMove(const Board &board, const Settings &settings)
{
	const auto validMoves = board.listValidMoves<RootMove>();
	assert(!validMoves.empty());
	
	for (const RootMove &move : validMoves)
		if (move.board.state == State::WINNER_WHITE || move.board.state == State::WINNER_BLACK)
			return move;

	// Apply Settings
	short depth = settings.getBaseSearchDepth() - 1;
	const auto threadCount = settings.getThreadCount();
	s_QuiescenceSearchEnabled = settings.performQuiescenceSearch();

	// If the Transposition Table wasn't resized, increment its age
	if (!s_SearchCache.setSize(settings.getCacheTableSizeMb()))
		s_SearchCache.incrementAge();

	// Update ThreadPool size if needed
	if (s_ThreadPool.threadCount() != threadCount)
		s_ThreadPool.updateThreadCount(threadCount);

	if (board.getPhase() == Phase::ENDING)
		++depth;

	return negaMaxRoot(validMoves, std::min<unsigned>(threadCount, validMoves.size()), depth);
}

short Search::getBestMoveFound()
{
	return s_BestMoveFound;
}

RootMove Search::negaMaxRoot(const std::vector<RootMove> &validMoves, const unsigned jobCount, const short ply)
{
	std::vector<std::future<void>> futures;
	futures.reserve(jobCount);
	std::mutex mutex;
	
	RootMove bestMove = validMoves.front();
	auto currentMove = validMoves.begin();
	const auto lastMove = validMoves.end();
	short alpha = VALUE_MIN;

	const auto worker = [&] {
		mutex.lock();

		while (currentMove != lastMove)
		{
			// Make a copy of the needed variables while locked
			const RootMove &move = *currentMove;
			++currentMove;
			const short beta = -alpha;

			mutex.unlock(); // Process the result asynchronously
			const short result = -negaMax(move.board, ply, VALUE_MIN, beta, 1, false);
			mutex.lock();

			if (result > alpha)
			{
				alpha = result;
				bestMove = move;
			}
		}

		mutex.unlock();
	};

	// Initially create only use thread
	futures.emplace_back(s_ThreadPool.submitTask(worker));

	if (jobCount > 1)
	{
		// Wait until an alpha bound has been found
		while (alpha == VALUE_MIN)
			std::this_thread::yield();

		// After an alpha bound was found start searching using multiple threads
		for (unsigned i = 1u; i < jobCount; ++i)
			futures.emplace_back(s_ThreadPool.submitTask(worker));
	}

	for (auto &future : futures)
		future.get(); // Wait for the search to finish

	s_BestMoveFound = oppositeColor(validMoves.front().board.colorToMove) ? alpha : -alpha;

	return bestMove;
}

short Search::negaMax(const Board &board, const short ply, short alpha, short beta, const short depth, const bool moveCountPruning)
{
	if (board.state == State::DRAW)
		return 0;
	if (ply <= 0)
	{
		if (ply == -1 || board.state != State::WHITE_IN_CHECK || board.state != State::BLACK_IN_CHECK)
		{
			// Switch to Quiescence Search if enabled
			return s_QuiescenceSearchEnabled ? quiescence(board, alpha, beta) : sideToMove(board);
		}

		// If in check, allow the search to be extended to ply -1
	}

	const short originalAlpha = alpha;
	if (const SearchCache cache = s_SearchCache[board.zKey];
		cache.age == s_SearchCache.currentAge()
		&& board.zKey == cache.key
		&& board.score == cache.boardScore
		&& cache.ply == ply)
	{
		if (cache.flag == Flag::EXACT)
			return cache.value;
		if (cache.flag == Flag::ALPHA)
			alpha = std::max(alpha, cache.value);
		else if (cache.flag == Flag::BETA)
			beta = std::min(beta, cache.value);

		if (alpha >= beta)
			return cache.value;
	}

	const auto validMoves = board.listValidMoves<Board>();
	short bestScore = VALUE_MIN;
	short movesCount = 0;

	Stats::incrementNodesSearched();
	Stats::incrementNodesGenerated(validMoves.size());

	for (const Board &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
		{
			// Mate Pruning
			const short mateValue = VALUE_WINNER_WHITE - depth;
			if (mateValue < beta)
			{
				beta = mateValue;
				if (alpha >= mateValue)
				{
					bestScore = mateValue;
					break;
				}
			}

			if (mateValue > alpha)
			{
				alpha = mateValue;
				if (beta <= mateValue)
				{
					bestScore = mateValue;
					break;
				}
			}

			if (bestScore > mateValue)
				bestScore = mateValue;
			continue;
		}

		short moveScore = alpha + 1;

		// Late Move Reductions
		if (!moveCountPruning &&
			movesCount > 4 &&
			depth >= 3 &&
			ply >= 3 &&
			move.state != State::WHITE_IN_CHECK &&
			move.state != State::BLACK_IN_CHECK &&
			!move.isCapture &&
			!move.isPromotion)
			moveScore = -negaMax(move, ply - 2, -moveScore, -alpha, depth + 1, true);

		if (moveScore > alpha)
			moveScore = -negaMax(move, ply - 1, -beta, -alpha, depth + 1, false);

		if (moveScore > bestScore)
			bestScore = moveScore;

		// Alpha-Beta Pruning
		if (bestScore >= beta)
			break;
		if (bestScore > alpha)
			alpha = bestScore;

		++movesCount;
	}

	// Store the result in the transposition table
	Flag flag = Flag::EXACT;
	if (bestScore < originalAlpha)
		flag = Flag::ALPHA;
	else if (bestScore > beta)
		flag = Flag::BETA;

	s_SearchCache.insert({ board.zKey, board.score, bestScore, ply, flag });

	if (bestScore > alpha)
		alpha = bestScore;

	return alpha;
}

short Search::quiescence(const Board &board, short alpha, const short beta)
{
	if (board.state == State::DRAW)
		return 0;

	const short standPat = sideToMove(board);

	if (standPat >= beta)
		return standPat;
	if (standPat > alpha)
		alpha = standPat;

	// Delta Pruning
	if (board.getPhase() != Phase::ENDING) // Turn it off in the Endgame
	{
		constexpr short QUEEN_VALUE = 2538;
		constexpr short PAWN_VALUE = 128;

		short bigDelta = QUEEN_VALUE;
		if (board.isPromotion)
			bigDelta += QUEEN_VALUE - PAWN_VALUE;

		if (standPat < alpha - bigDelta)
			return alpha;
	}

	const auto validMoves = board.listQuiescenceMoves();

	Stats::incrementNodesSearched();
	Stats::incrementNodesGenerated(validMoves.size());

	for (const Board &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const short moveScore = -quiescence(move, -beta, -alpha);

		if (moveScore >= beta)
			return moveScore;
		if (moveScore > alpha)
			alpha = moveScore;
	}

	return alpha;
}

short Search::negaScout(const Board &board, const short ply, short alpha, const short beta, const bool isWhite, const short depth)
{
	if (board.state == State::DRAW)
		return 0;
	if (ply == 0)
	{
		if (s_QuiescenceSearchEnabled)
			return quiescence(board, alpha, beta);
		return sideToMove(board);
	}

	const auto validMoves = board.listValidMoves<Board>();
	short bestScore = VALUE_MIN;
	short n = beta;

	for (const auto &move : validMoves)
	{
		if (move.state == State::WINNER_WHITE || move.state == State::WINNER_BLACK)
			return VALUE_WINNER_WHITE;

		const short moveScore = -negaScout(move, ply - 1, -n, -alpha, !isWhite, depth + 1);

		if (moveScore > bestScore)
			bestScore = (n == beta || ply <= 2) ? moveScore : -negaScout(move, ply - 1, -beta, -bestScore, !isWhite, depth + 1);

		if (bestScore > alpha)
			alpha = bestScore;

		if (alpha >= beta)
			break; // return alpha;

		n = alpha + 1;
	}

	return bestScore;
}

inline short Search::sideToMove(const Board &board)
{
	const short value = Evaluation::evaluate(board);
	return board.colorToMove ? value : -value;
}
