#include <jni.h>

#include "Log.h"
#include "Cache.h"
#include "AndroidBuffer.h"

#include "chess/BoardManager.h"
#include "chess/Stats.h"
#include "chess/Tests.h"
#include "chess/persistence/MovesPersistence.h"
#include "chess/algorithm/Evaluation.h"
#include "chess/algorithm/MoveGen.h"
#include "chess/algorithm/Search.h"
#include "chess/polyglot/PolyBook.h"

#define external extern "C"

static JavaVM *jvm = nullptr;
static jobject viewModelInstance = nullptr;

const BoardManager::BoardChangedListener listener = [](GameState state)
{
	JNIEnv *env;
	int getEnvStat = jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->AttachCurrentThread(&env, nullptr);
		LOGD("ChessCpp", "Attached to Thread");
	}

	env->ExceptionClear();

	const static auto callbackId = env->GetMethodID(Cache::viewModelClass, "callback","(I)V");
	env->CallVoidMethod(viewModelInstance, callbackId, static_cast<jint>(state));

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

	std::cout.rdbuf(new AndroidBuffer);

	Cache::createCaches(env);

	return JNI_VERSION_1_6;
}

external JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *)
{
	LOGI("ChessCpp", "JNI_OnUnload");

	JNIEnv *env;
	vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);

	// Clean the caches
	env->DeleteGlobalRef(viewModelInstance);
	viewModelInstance = nullptr;
	Cache::cleanCaches(env);

	delete std::cout.rdbuf(nullptr);

	jvm = nullptr;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_GameManager_initBoardNative(JNIEnv *pEnv, jobject instance,
														 jboolean isPlayerWhite)
{
	// TODO REMOVE
	pEnv->ExceptionClear();

	if (!pEnv->IsSameObject(viewModelInstance, instance))
	{
		LOGD("ChessCpp", "initBoardNative-GameManager");
		pEnv->DeleteGlobalRef(viewModelInstance);
		viewModelInstance = pEnv->NewGlobalRef(instance);

		BoardManager::initBoardManager(listener);
	}

	BoardManager::initBoardManager(listener, isPlayerWhite);
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_ChessViewModel_initBoardNative(JNIEnv *pEnv, jobject instance,
		jboolean playerPlayingWhite)
{
	pEnv->ExceptionClear();

	if (!pEnv->IsSameObject(viewModelInstance, instance))
	{
        LOGD("ChessCpp", "initBoardNative");
        pEnv->DeleteGlobalRef(viewModelInstance);
		viewModelInstance = pEnv->NewGlobalRef(instance);

        BoardManager::initBoardManager(listener);
	}

	BoardManager::initBoardManager(listener, playerPlayingWhite);
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

	const static auto constructorId = pEnv->GetMethodID(Cache::indexedPieceClass, "<init>", "(IIBZ)V");

	auto &&pieceList = BoardManager::getIndexedPieces();

	jobjectArray array =
		pEnv->NewObjectArray(static_cast<jsize>(pieceList.size()), Cache::indexedPieceClass, nullptr);

	for (size_t i{}; i < pieceList.size(); ++i)
	{
		const auto &it = pieceList[i];
		jobject obj = pEnv->NewObject(Cache::indexedPieceClass, constructorId, it.id, it.square, it.pieceType, it.pieceColor);
		pEnv->SetObjectArrayElement(array, i, obj);
	}

	return array;
}

external JNIEXPORT jobjectArray JNICALL
Java_net_theluckycoder_chess_Native_getPossibleMoves(JNIEnv *pEnv, jobject, jbyte square)
{
	pEnv->ExceptionClear();

	const static auto constructorId = pEnv->GetMethodID(Cache::moveClass, "<init>", "(IBBBBBB)V");

	const auto possibleMoves = BoardManager::getPossibleMoves(toSquare(square));

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
												jboolean quietSearch,
												jint threadCount,
												jint hashSizeMb)
{
	BoardManager::setSettings(Settings{ searchDepth,
										static_cast<u32>(threadCount),
										static_cast<u32>(hashSizeMb),
										static_cast<bool>(quietSearch) });
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
Java_net_theluckycoder_chess_Native_getCurrentFen(JNIEnv *pEnv, jobject)
{
	const auto fenString = BoardManager::getBoard().getFen();

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
Java_net_theluckycoder_chess_Native_initBook(JNIEnv *pEnv, jobject, jstring bookPath)
{
	const char *nativeString = pEnv->GetStringUTFChars(bookPath, nullptr);
	PolyBook::initBook(nativeString);
	LOGV("Book", "Initialized");

	pEnv->ReleaseStringUTFChars(bookPath, nativeString);
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
