#include <jni.h>

#include <chrono>

#include "Log.h"
#include "Cache.h"
#include "external.h"

#include "chess/BoardManager.h"
#include "chess/Stats.h"
#include "chess/data/Board.h"
#include "chess/persistence/MovesPersistence.h"
#include "chess/algorithm/Search.h"

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

external JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void */*reserved*/)
{
	LOGI("ChessCpp", "JNI_OnLoad");

	JNIEnv *env;
	if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
		return -1;

	jvm = vm;

	Cache::createCaches(env);

	return JNI_VERSION_1_6;
}

external JNIEXPORT void JNI_OnUnload(JavaVM *vm, void */*reserved*/)
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
Java_net_theluckycoder_chess_Native_isWorking(JNIEnv */*pEnv*/, jclass /*type*/)
{
	return static_cast<jboolean>(BoardManager::isWorking());
}

external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_isPlayerWhite(JNIEnv */*pEnv*/, jclass /*type*/)
{
	return static_cast<jboolean>(BoardManager::isPlayerWhite());
}

// region Stats

external JNIEXPORT jdouble JNICALL
Java_net_theluckycoder_chess_Native_getSearchTime(JNIEnv */*pEnv*/, jclass /*type*/)
{
	return static_cast<jdouble>(Stats::getElapsedTime());
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getCurrentBoardValue(JNIEnv */*pEnv*/, jclass /*type*/)
{
	return static_cast<jint>(BoardManager::getBoard().score);
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getBestMoveFound(JNIEnv */*pEnv*/, jclass /*type*/)
{
	return static_cast<jint>(Search::getBestMoveFound());
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_getAdvancedStats(JNIEnv *pEnv, jclass /*type*/)
{
	return pEnv->NewStringUTF(Stats::formatStats('\n').c_str());
}

// endregion Stats

external JNIEXPORT _jobjectArray *JNICALL
Java_net_theluckycoder_chess_Native_getPieces(JNIEnv *pEnv, jclass /*type*/)
{
	pEnv->ExceptionClear();

	const static auto constructorId = pEnv->GetMethodID(Cache::pieceClass, "<init>", "(IIB)V");

	const auto pieces = BoardManager::getBoard().getAllPieces();
	auto *array = pEnv->NewObjectArray(static_cast<jsize>(pieces.size()), Cache::pieceClass,
									   nullptr);

	jsize i = 0;
	for (const auto &it : pieces)
	{
		const Pos &pos = it.first;
		auto pieceType = static_cast<jbyte>(it.second.type);
		if (it.second.color == BLACK)
			pieceType += 6;

		jobject obj = pEnv->NewObject(Cache::pieceClass, constructorId, pos.x, pos.y, pieceType);
		pEnv->SetObjectArrayElement(array, i, obj);

		++i;
	}

	return array;
}

external JNIEXPORT _jobjectArray *JNICALL
Java_net_theluckycoder_chess_Native_getPossibleMoves(JNIEnv *pEnv, jclass /*type*/, jobject dest)
{
	pEnv->ExceptionClear();

	const static auto constructorId = pEnv->GetMethodID(Cache::posClass, "<init>", "(II)V");

	const Pos pos(getInt(pEnv, Cache::posClass, dest, "x"),
				  getInt(pEnv, Cache::posClass, dest, "y"));
	const auto possibleMoves = BoardManager::getPossibleMoves(pos);
	auto *result = pEnv->NewObjectArray(static_cast<jsize>(possibleMoves.size()), Cache::posClass,
										nullptr);

	for (unsigned i = 0; i < possibleMoves.size(); ++i)
	{
		jobject obj = pEnv->NewObject(Cache::posClass,
									  constructorId,
									  static_cast<int>(possibleMoves[i].x),
									  static_cast<int>(possibleMoves[i].y));
		pEnv->SetObjectArrayElement(result, i, obj);
	}

	return result;
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_enableStats(JNIEnv */*pEnv*/, jclass /*type*/, jboolean enabled)
{
	Stats::setEnabled(enabled);
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_setSettings(JNIEnv */*pEnv*/, jclass /*type*/,
												jint baseSearchDepth,
												jint threadCount, jint cacheSizeMb,
												jboolean performQuiescenceSearch)
{
	BoardManager::setSettings(Settings(static_cast<short>(baseSearchDepth),
									   static_cast<unsigned int>(threadCount),
									   static_cast<unsigned int>(cacheSizeMb),
									   static_cast<bool>(performQuiescenceSearch)));
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_movePiece(JNIEnv */*pEnv*/, jclass /*type*/,
											  jbyte selectedX, jbyte selectedY, jbyte destX,
											  jbyte destY)
{
	BoardManager::movePiece(
		toSquare(static_cast<byte>(selectedX), static_cast<byte>(selectedY)),
		toSquare(static_cast<byte>(destX), static_cast<byte>(destY)));
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_undoMoves(JNIEnv */*pEnv*/, jclass /*type*/)
{
	BoardManager::undoLastMoves();
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_loadMoves(JNIEnv *pEnv, jclass /*type*/, jstring moves)
{
	const char *nativeString = pEnv->GetStringUTFChars(moves, nullptr);
	const MovesPersistence savedMoves(nativeString);

	BoardManager::loadGame(savedMoves.getMoves(), savedMoves.isPlayerWhite());

	pEnv->ReleaseStringUTFChars(moves, nativeString);
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_saveMoves(JNIEnv *pEnv, jclass /*type*/)
{
	const std::string string = MovesPersistence::saveToString(BoardManager::getMovesHistory(),
															  BoardManager::isPlayerWhite());
	return pEnv->NewStringUTF(string.c_str());
}

static U64 perft(const Board &board, const int depth)
{
	if (depth == 0) return 1;

	const auto validMoves = board.listValidMoves<Board>();

	if (depth == 1) return validMoves.size();

	U64 nodes{};

	for (const auto &move : validMoves)
		nodes += perft(move, depth - 1);

	return nodes;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_perft(JNIEnv */*pEnv*/, jclass /*type*/, jint depth)
{
	using namespace std::chrono;

	constexpr auto TAG = "Perft Test";
	constexpr std::array<U64, 7> perftResults{
		1, 20, 400, 8902, 197281, 4865609, 119060324
	};
	constexpr int maxSize = static_cast<int>(perftResults.size());

	if (depth > maxSize)
		depth = maxSize; // Otherwise it will take too long to compute

	Board board;
	board.initDefaultBoard();

	for (int i = 0; i <= depth; ++i)
	{
		LOGV(TAG, "Starting Depth %d Test", i);

		const auto startTime = high_resolution_clock::now();
		const U64 nodesCount = perft(board, i);
		const auto endTime = high_resolution_clock::now();

		const auto timeNeeded = duration<double, std::milli>(endTime - startTime).count();

		LOGV(TAG, "Time needed: %lf", timeNeeded);
		LOGV(TAG, "Nodes count: %llu/%llu", nodesCount, perftResults[i]);
		if (nodesCount != perftResults[i])
			LOGE(TAG, "Nodes count do not match at depth %d", i);
	}

	LOGV(TAG, "Test Finished");
}
