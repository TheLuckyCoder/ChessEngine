#include <jni.h>

#include "Log.h"
#include "Cache.h"

#include "chess/BoardManager.h"
#include "chess/Stats.h"
#include "chess/Tests.h"
#include "chess/persistence/MovesPersistence.h"
#include "chess/algorithm/Evaluation.h"
#include "chess/algorithm/MoveGen.h"
#include "chess/algorithm/Search.h"

#define external extern "C"

static JavaVM *jvm = nullptr;
static jobject gameManagerInstance = nullptr;

const BoardManager::PieceChangeListener listener = [](GameState state, bool shouldRedraw,
													  const std::vector<std::pair<u8, u8>> &moved)
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
		const u8 startSq = moved[i].first;
		const u8 destSq = moved[i].second;

		jobject obj = env->NewObject(Cache::posPairClass, constructorId,
									 fileOf(startSq), rankOf(startSq), fileOf(destSq), rankOf(destSq));

		env->SetObjectArrayElement(result, i, obj);
	}

	const static auto callbackId = env->GetMethodID(Cache::gameManagerClass, "callback",
													"(IZ[Lnet/theluckycoder/chess/model/PosPair;)V");
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
	gameManagerInstance = nullptr;
	Cache::cleanCaches(env);

	jvm = nullptr;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_GameManager_initBoardNative(JNIEnv *pEnv, jobject instance,
														 jboolean isPlayerWhite)
{
	pEnv->ExceptionClear();

	if (!pEnv->IsSameObject(gameManagerInstance, instance))
	{
		LOGD("ChessCpp", "initBoardNative");
		pEnv->DeleteGlobalRef(gameManagerInstance);
		gameManagerInstance = pEnv->NewGlobalRef(instance);

		BoardManager::initBoardManager(listener);
	}

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
	return static_cast<jint>(Evaluation::value(BoardManager::getBoard()));
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

	const static auto constructorId = pEnv->GetMethodID(Cache::pieceClass, "<init>", "(IB)V");

	std::vector<std::pair<u8, u8>> pieceList;
	pieceList.reserve(32);

	const auto &board = BoardManager::getBoard();
	for (u8 color = BLACK; color <= WHITE; ++color)
	{
		for (u8 pieceType = PAWN; pieceType <= KING; ++pieceType)
		{
			const Piece piece{ PieceType(pieceType), Color(color) };
			for (u8 pieceNumber{}; pieceNumber < board.pieceCount[piece]; ++pieceNumber)
			{
				const u8 sq = board.pieceList[piece][pieceNumber];

				pieceList.emplace_back(sq, color == BLACK ? u8(pieceType + 6u) : pieceType);
			}
		}
	}

	jobjectArray array =
		pEnv->NewObjectArray(static_cast<jsize>(pieceList.size()), Cache::pieceClass, nullptr);

	for (size_t i{}; i < pieceList.size(); ++i)
	{
		const auto &it = pieceList[i];
		jobject obj =
			pEnv->NewObject(Cache::pieceClass, constructorId, it.first, it.second);
		pEnv->SetObjectArrayElement(array, i, obj);
	}

	return array;
}

external JNIEXPORT jobjectArray JNICALL
Java_net_theluckycoder_chess_Native_getPossibleMoves(JNIEnv *pEnv, jobject, jbyte square)
{
	pEnv->ExceptionClear();

	const static auto constructorId = pEnv->GetMethodID(Cache::moveClass, "<init>", "(IBBBBBB)V");

	const auto possibleMoves = BoardManager::getPossibleMoves(u8(square));
	LOGD("Possible Moves", "%d", i32(possibleMoves.size()));

	jobjectArray result =
		pEnv->NewObjectArray(static_cast<jsize>(possibleMoves.size()), Cache::moveClass, nullptr);

	int i{};
	std::for_each(possibleMoves.begin(), possibleMoves.end(), [&](const Move &move)
	{
		jobject obj = pEnv->NewObject(Cache::moveClass, constructorId,
									  move.getContents(), move.from(), move.to(), move.piece(),
									  move.capturedPiece(), move.promotedPiece(), move.flags());
		pEnv->SetObjectArrayElement(result, i++, obj);
	});

	return result;
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_enableStats(JNIEnv *, jobject, jboolean enabled)
{
	Stats::setEnabled(enabled);
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_setSettings(JNIEnv *, jobject, jint searchDepth,
												jint threadCount,
												jint cacheSizeMb,
												jboolean performQuiescenceSearch)
{
	BoardManager::setSettings(Settings{ searchDepth,
										static_cast<u32>(threadCount),
										static_cast<u32>(cacheSizeMb),
										static_cast<bool>(performQuiescenceSearch) });
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_makeMove(JNIEnv *, jobject, jint move)
{
	BoardManager::makeMove(Move{ static_cast<u32>(move) });
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_forceMove(JNIEnv *, jobject)
{
	BoardManager::forceMove();
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_stopSearch(JNIEnv *, jobject)
{
	Search::stopSearch();
}


external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_undoMoves(JNIEnv *, jobject)
{
	return static_cast<jboolean>(BoardManager::undoLastMoves());
}

external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_loadFen(JNIEnv *pEnv, jobject, jboolean playerWhite,
											jstring fenPosition)
{
	const char *nativeString = pEnv->GetStringUTFChars(fenPosition, nullptr);

	const bool loaded = BoardManager::loadGame(playerWhite, nativeString);

	pEnv->ReleaseStringUTFChars(fenPosition, nativeString);

	return static_cast<jboolean>(loaded);
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
Java_net_theluckycoder_chess_Native_getFen(JNIEnv *pEnv, jobject)
{
	const auto fenString = BoardManager::exportFen();

	return pEnv->NewStringUTF(fenString.c_str());
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_saveMoves(JNIEnv *pEnv, jobject)
{
	const auto moves = BoardManager::getMovesHistory();
	if (moves.empty())
		return nullptr;

	const auto string = MovesPersistence::saveToString(moves, BoardManager::isPlayerWhite());
	return pEnv->NewStringUTF(string.c_str());
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_perftTest(JNIEnv *, jobject)
{
	Tests::runPerftTests();
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_evaluationTest(JNIEnv *pEnv, jobject)
{
	const auto testResults = Tests::runEvaluationTests();
	if (testResults.empty())
		return nullptr;

	return pEnv->NewStringUTF(testResults.c_str());
}
