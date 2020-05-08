#include <jni.h>

#include <chrono>

#include "Log.h"
#include "Cache.h"
#include "Perft.h"

#include "chess/BoardManager.h"
#include "chess/Stats.h"
#include "chess/persistence/MovesPersistence.h"
#include "chess/algorithm/Evaluation.h"
#include "chess/algorithm/MoveGen.h"
#include "chess/data/Pos.h"

#define external extern "C"

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
		const Pos startPos{ moved[i].first };
		const Pos destPos{ moved[i].second };
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
	return static_cast<jdouble>(Stats::getElapsedMs());
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getCurrentBoardValue(JNIEnv *, jobject)
{
	return static_cast<jint>(Evaluation::evaluate(BoardManager::getBoard()).value);
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

				pieceList.emplace_back(Pos{ sq }, color == BLACK ? byte(pieceType + 6u) : pieceType);
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
	const auto moves = BoardManager::getMovesHistory();
	if (!moves.empty())
	{
		const auto string = MovesPersistence::saveToString(moves, BoardManager::isPlayerWhite());
		return pEnv->NewStringUTF(string.c_str());
	}
	return nullptr;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_perft(JNIEnv *, jobject)
{
	Attacks::init();

	// Position 1
	perftTest("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {
		1, 20, 400, 8902, 197281, 4865609, 119060324
	});
	// Position 2
	perftTest("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", {
		1, 48, 2039, 97862, 4085603, 193690690, 8031647685
	});
	// Position 3
	perftTest("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", {
		1, 14, 191, 2812, 43238, 674624, 11030083, 178633661
	});
	// Position 4
	perftTest("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {
		1, 6, 264, 9467, 422333, 15833292, 706045033
	});
	// Position 5
	perftTest("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ", {
		1, 44, 1486, 62379, 2103487, 89941194
	});
	// Position 6
	perftTest("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ", {
		1, 46, 2079, 89890, 3894594, 164075551, 6923051137
	});
}
