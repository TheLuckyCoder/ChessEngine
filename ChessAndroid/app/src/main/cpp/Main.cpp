#include <jni.h>

#include <chrono>

#include "Log.h"
#include "Cache.h"
#include "external.h"

#include "chess/BoardManager.h"
#include "chess/Stats.h"
#include "chess/persistence/MovesPersistence.h"
#include "chess/algorithm/Evaluation.h"
#include "chess/algorithm/MoveGen.h"

static JavaVM *jvm = nullptr;
static jobject gameManagerInstance;

const BoardManager::PieceChangeListener listener = [](State state, bool shouldRedraw,
													  const std::vector<std::pair<byte, byte>> &moved)
{
	JNIEnv *env;
	int getEnvStat = jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->AttachCurrentThread(&env, nullptr);
		LOGD("ChessCpp", "Attached to Thread");
	}

	env->ExceptionClear();

	jobjectArray result = env->NewObjectArray(static_cast<jsize>(moved.size()), Cache::posPairClass,
											  nullptr);

	const static auto constructorId = env->GetMethodID(Cache::posPairClass, "<init>", "(IIII)V");

	for (unsigned i = 0; i < moved.size(); ++i)
	{
		const Pos &startPos = Pos(moved[i].first);
		const Pos &destPos = Pos(moved[i].second);
		jobject obj = env->NewObject(Cache::posPairClass, constructorId,
									 startPos.x, startPos.y, destPos.x, destPos.y);

		env->SetObjectArrayElement(result, i, obj);
	}

	const static auto callbackId = env->GetMethodID(Cache::gameManagerClass, "callback",
													"(IZ[Lnet/theluckycoder/chess/PosPair;)V");
	env->CallVoidMethod(gameManagerInstance, callbackId,
						static_cast<jint>(state), static_cast<jboolean>(shouldRedraw), result);

	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->DetachCurrentThread();
		LOGD("ChessCpp", "Detached from Thread");
	}
};

external JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
	LOGI("ChessCpp", "JNI_OnLoad");

	JNIEnv *env;
	if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
		return -1;

	jvm = vm;

	Cache::createCaches(env);

	return JNI_VERSION_1_6;
}

external JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *)
{
	LOGI("ChessCpp", "JNI_OnUnload");

	JNIEnv *env;
	vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);

	// Clean the caches
	env->DeleteGlobalRef(gameManagerInstance);
	Cache::cleanCaches(env);

	jvm = nullptr;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_GameManager_initBoardNative(JNIEnv *pEnv, jobject instance,
														 jboolean restartGame,
														 jboolean isPlayerWhite)
{
	static bool boardManagerInitialized = false;
	pEnv->ExceptionClear();

	if (!boardManagerInitialized)
	{
		pEnv->DeleteGlobalRef(gameManagerInstance);
		gameManagerInstance = pEnv->NewGlobalRef(instance);
		Cache::gameManagerClass = Cache::cacheClass(pEnv,
													pEnv->GetObjectClass(gameManagerInstance));

		boardManagerInitialized = true;
		BoardManager::initBoardManager(listener);
	}

	if (restartGame)
		BoardManager::initBoardManager(listener, isPlayerWhite);
}

// Native Class

external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_isWorking(JNIEnv *, jobject)
{
	return static_cast<jboolean>(BoardManager::isWorking());
}

external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_isPlayerWhite(JNIEnv *, jobject)
{
	return static_cast<jboolean>(BoardManager::isPlayerWhite());
}

// region Stats

external JNIEXPORT jdouble JNICALL
Java_net_theluckycoder_chess_Native_getSearchTime(JNIEnv *, jobject)
{
	return static_cast<jdouble>(Stats::getElapsedTime());
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getCurrentBoardValue(JNIEnv *, jobject)
{
	return static_cast<jint>(Evaluation::evaluate(BoardManager::getBoard()));
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getBestMoveFound(JNIEnv *, jobject)
{
	return 0;//static_cast<jint>(Search::getBestMoveFound());
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_getAdvancedStats(JNIEnv *pEnv, jobject)
{
	return pEnv->NewStringUTF(Stats::formatStats('\n').c_str());
}

// endregion Stats

external JNIEXPORT jobjectArray JNICALL
Java_net_theluckycoder_chess_Native_getPieces(JNIEnv *pEnv, jobject)
{
	pEnv->ExceptionClear();

	const static auto constructorId = pEnv->GetMethodID(Cache::pieceClass, "<init>", "(IIB)V");

	std::vector<std::pair<Pos, byte>> pieceList;
	pieceList.reserve(32);

	const auto &board = BoardManager::getBoard();
	for (byte color = BLACK; color <= WHITE; ++color)
	{
		for (byte pieceType = PAWN; pieceType <= KING; ++pieceType)
		{
			const Piece piece{ PieceType(pieceType), Color(color) };
			for (byte pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
			{
				const byte sq = board.pieceList[piece][pieceNumber];
				const Pos pos(sq);

				pieceList.emplace_back(pos, color == BLACK ? byte(pieceType + 6u) : pieceType);
			}
		}
	}

	jobjectArray array =
		pEnv->NewObjectArray(static_cast<jsize>(pieceList.size()), Cache::pieceClass, nullptr);

	for (size_t i{}; i < pieceList.size(); ++i)
	{
		const auto &it = pieceList[i];
		jobject obj =
			pEnv->NewObject(Cache::pieceClass, constructorId, int(it.first.x), int(it.first.y),
							it.second);
		pEnv->SetObjectArrayElement(array, i, obj);
	}

	return array;
}

external JNIEXPORT jlongArray JNICALL
Java_net_theluckycoder_chess_Native_getPossibleMoves(JNIEnv *pEnv, jobject, jbyte square)
{
	pEnv->ExceptionClear();

	const auto possibleMoves = BoardManager::getPossibleMoves(byte(square));
	std::vector<jlong> longMoves;
	longMoves.reserve(possibleMoves.size());

	std::transform(possibleMoves.begin(), possibleMoves.end(), std::back_inserter(longMoves),
				   [](const Move &c)
				   { return jlong(c.getContents()); });

	auto result = pEnv->NewLongArray(jsize(possibleMoves.size()));

	pEnv->SetLongArrayRegion(result, 0, longMoves.size(), longMoves.data());

	return result;
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_enableStats(JNIEnv *, jobject, jboolean enabled)
{
	Stats::setEnabled(enabled);
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_setSettings(JNIEnv *, jobject, jint baseSearchDepth,
												jint threadCount,
												jint cacheSizeMb,
												jboolean performQuiescenceSearch)
{
	BoardManager::setSettings(Settings(static_cast<unsigned int>(baseSearchDepth),
									   static_cast<unsigned int>(threadCount),
									   static_cast<unsigned int>(cacheSizeMb),
									   static_cast<bool>(performQuiescenceSearch)));
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_makeMove(JNIEnv *, jobject, jlong move)
{
	BoardManager::makeMove(Move(static_cast<unsigned>(move & UINT32_MAX), 0));
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_forceMove(JNIEnv *, jobject)
{
	BoardManager::forceMove();
}


external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_undoMoves(JNIEnv *, jobject)
{
	return static_cast<jboolean>(BoardManager::undoLastMoves());
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_loadMoves(JNIEnv *pEnv, jobject, jstring moves)
{
	const char *nativeString = pEnv->GetStringUTFChars(moves, nullptr);
	const MovesPersistence savedMoves(nativeString);

	BoardManager::loadGame(savedMoves.getMoves(), savedMoves.isPlayerWhite());

	pEnv->ReleaseStringUTFChars(moves, nativeString);
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_saveMoves(JNIEnv *pEnv, jobject)
{
	const std::string string = MovesPersistence::saveToString(BoardManager::getMovesHistory(),
															  BoardManager::isPlayerWhite());
	return pEnv->NewStringUTF(string.c_str());
}


static U64 perft(Board &board, const unsigned depth)
{
	if (board.fiftyMoveRule == 100)
		return 0;

	if (depth == 0)
		return 1;

	MoveList<ALL> moveList(board);

	size_t legalCount{};

	for (const Move &move : moveList)
	{
		if (!board.makeMove(move))
			continue;

		legalCount += perft(board, depth - 1);
		board.undoMove();
	}

	return legalCount;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_perft(JNIEnv *, jobject, jint depth)
{
	using namespace std::chrono;

	Board board;
	board.setToFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");

	constexpr auto TAG = "Perft Test: ";
	constexpr std::array<U64, 8> perftResults{
		1, 14, 191, 2812, 43238, 674624, 11030083, 178633661
	};

	constexpr int maxSize = static_cast<int>(perftResults.size());

	if (depth > maxSize)
		depth = maxSize; // Otherwise it will take too long to compute

	for (unsigned i = 0; i < perftResults.size(); ++i)
	{
		LOGV(TAG, "Starting Depth %d Test", i);

		const auto startTime = high_resolution_clock::now();
		const U64 nodesCount = perft(board, i);

		const auto currentTime = high_resolution_clock::now();
		const double timeNeeded = duration<double, std::milli>(currentTime - startTime).count();

		LOGV(TAG, "Time needed: %lf", timeNeeded);
		LOGV(TAG, "Nodes count: %llu/%llu", nodesCount, perftResults[i]);
		if (nodesCount != perftResults[i])
			LOGE(TAG, "Nodes count do not match at depth %d", i);
	}

	LOGV(TAG, "Perft Test Finished");
}
