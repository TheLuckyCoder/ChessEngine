#include <jni.h>

#include <iostream>

#include "Log.h"
#include "JniCache.h"
#include "AndroidBuffer.h"

#include "chess/BoardManager.h"
#include "chess/Stats.h"
#include "chess/Tests.h"
#include "chess/algorithm/Evaluation.h"
#include "chess/MoveGen.h"
#include "chess/algorithm/Search.h"
#include "chess/polyglot/PolyBook.h"

#define external extern "C"

static constexpr auto TAG = "ChessCpp";
static JavaVM *jvm = nullptr;
static jobject viewModelInstance = nullptr;

const BoardManager::BoardChangedCallback boardChangedCallback = [](GameState state)
{
	JNIEnv *env;
	int getEnvStat = jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->AttachCurrentThread(&env, nullptr);
		LOGD(TAG, "Attached to Thread");
	}

	env->ExceptionClear();

	const static auto callbackId = env->GetMethodID(JniCache::viewModelClass, "boardChangedCallback", "(I)V");
	if (viewModelInstance == nullptr)
	{
		LOGE(TAG, "No ViewModel Instance found");
		return;
	}

	env->CallVoidMethod(viewModelInstance, callbackId, static_cast<jint>(state));

	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->DetachCurrentThread();
		LOGD(TAG, "Detached from Thread");
	}
};

external JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *)
{
	LOGI(TAG, "JNI_OnLoad");

	JNIEnv *env;
	if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
		return -1;

	jvm = vm;

	std::cout.rdbuf(new AndroidBuffer);

	JniCache::createCaches(env);

	return JNI_VERSION_1_6;
}

external JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *)
{
	LOGI(TAG, "JNI_OnUnload");

	JNIEnv *env;
	vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);

	// Clean the caches
	env->DeleteGlobalRef(viewModelInstance);
	viewModelInstance = nullptr;
	JniCache::cleanCaches(env);

	delete std::cout.rdbuf(nullptr);

	jvm = nullptr;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_ChessViewModel_initBoardNative(JNIEnv *pEnv, jobject instance, jboolean playerPlayingWhite)
{
	pEnv->ExceptionClear();

	if (!pEnv->IsSameObject(viewModelInstance, instance))
	{
		LOGD(TAG, "initBoardNative");
		if (viewModelInstance)
			pEnv->DeleteGlobalRef(viewModelInstance);

		viewModelInstance = pEnv->NewGlobalRef(instance);

		BoardManager::initBoardManager(boardChangedCallback);
	}

	BoardManager::initBoardManager(boardChangedCallback, playerPlayingWhite);
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

external JNIEXPORT jobjectArray JNICALL
Java_net_theluckycoder_chess_Native_getPieces(JNIEnv *pEnv, jobject)
{
	pEnv->ExceptionClear();

	const static auto constructorId = pEnv->GetMethodID(JniCache::indexedPieceClass, "<init>", "(IIBZ)V");

	auto &&pieceList = BoardManager::getMovesStack().getIndexedPieces();

	jobjectArray array =
		pEnv->NewObjectArray(static_cast<jsize>(pieceList.size()), JniCache::indexedPieceClass, nullptr);

	for (size_t i{}; i < pieceList.size(); ++i)
	{
		const auto &it = pieceList[i];
		jobject obj = pEnv->NewObject(JniCache::indexedPieceClass, constructorId, it.id, it.square, it.pieceType,
									  it.pieceColor);
		pEnv->SetObjectArrayElement(array, i, obj);
	}

	return array;
}

external JNIEXPORT jobjectArray JNICALL
Java_net_theluckycoder_chess_Native_getPossibleMoves(JNIEnv *pEnv, jobject, jbyte square)
{
	const static auto constructorId = pEnv->GetMethodID(JniCache::moveClass, "<init>", "(IBBBBBB)V");

	const auto possibleMoves = BoardManager::getPossibleMoves(toSquare(square));

	jobjectArray result =
		pEnv->NewObjectArray(static_cast<jsize>(possibleMoves.size()), JniCache::moveClass, nullptr);

	usize i{};
	for (auto &&move : possibleMoves)
	{
		jobject obj = pEnv->NewObject(JniCache::moveClass, constructorId,
									  move.getContents(), move.from(), move.to(), move.piece(),
									  move.capturedPiece(), move.promotedPiece(), move.flags());
		pEnv->SetObjectArrayElement(result, i++, obj);
	}

	return result;
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_enableStats(JNIEnv *, jobject, jboolean enabled)
{
	Stats::setEnabled(enabled);
}


external JNIEXPORT jobject JNICALL
Java_net_theluckycoder_chess_Native_getSearchOptions(JNIEnv *pEnv, jobject)
{
	const static auto constructorId = pEnv->GetMethodID(JniCache::engineSettingsClass, "<init>", "(IIJIZ)V");

	const auto options = BoardManager::getSearchOptions();

	return pEnv->NewObject(JniCache::engineSettingsClass, constructorId,
						   options.depth(), options.threadCount(),
						   static_cast<jlong>(options.searchTime()), options.tableSizeMb(),
						   options.quietSearch());
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_setSearchOptions(JNIEnv *, jobject, jint searchDepth,
													 jboolean quietSearch,
													 jint threadCount,
													 jint hashSizeMb,
													 jlong searchTime)
{
	BoardManager::setSearchOptions(SearchOptions{ searchDepth,
												  static_cast<u32>(threadCount),
												  static_cast<u32>(hashSizeMb),
												  static_cast<bool>(quietSearch),
												  static_cast<u64>(searchTime) });
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_makeMove(JNIEnv *, jobject, jint move)
{
	BoardManager::makeMove(Move{ static_cast<u32>(move) });
	BoardManager::makeEngineMove();
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_forceMove(JNIEnv *, jobject)
{
	BoardManager::makeEngineMove();
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_stopSearch(JNIEnv *, jobject)
{
	Search::stopSearch();
}


external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_undoMoves(JNIEnv *, jobject)
{
	BoardManager::undoLastMoves();
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_redoMoves(JNIEnv *, jobject)
{
	BoardManager::redoLastMoves();
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
Java_net_theluckycoder_chess_Native_loadMoves(JNIEnv *pEnv, jobject, jboolean isPlayerWhite, jintArray moves)
{
	const usize size = pEnv->GetArrayLength(moves);
	auto elements = pEnv->GetIntArrayElements(moves, JNI_FALSE);

	std::vector<Move> vector;
	vector.reserve(size);

	for (usize i = 0; i < size; ++i)
		vector.emplace_back(elements[i]);

	BoardManager::loadGame(isPlayerWhite, vector);
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_getCurrentFen(JNIEnv *pEnv, jobject)
{
	const auto fenString = BoardManager::getBoard().getFen();

	return pEnv->NewStringUTF(fenString.c_str());
}

external JNIEXPORT jobjectArray JNICALL
Java_net_theluckycoder_chess_Native_getMovesHistory(JNIEnv *pEnv, jobject)
{
	const auto stack = BoardManager::getMovesStack();
	const static auto constructorId = pEnv->GetMethodID(JniCache::moveClass, "<init>", "(IBBBBBB)V");

	const auto result =
		pEnv->NewObjectArray(static_cast<jsize>(stack.size()), JniCache::moveClass, nullptr);

	usize i{};
	for (auto &&board : stack)
	{
		const Move move = board.getMove();
		jobject obj = pEnv->NewObject(JniCache::moveClass, constructorId,
									  move.getContents(), move.from(), move.to(), move.piece(),
									  move.capturedPiece(), move.promotedPiece(), move.flags());
		pEnv->SetObjectArrayElement(result, i++, obj);
	}

	return result;
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getCurrentMoveIndex(JNIEnv *, jobject)
{
	return BoardManager::getMovesStack().getCurrentIndex();
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
Java_net_theluckycoder_chess_Native_perftTests(JNIEnv *, jobject)
{
	Tests::runPerftTests();
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_evaluationTests(JNIEnv *pEnv, jobject)
{
	const auto testResults = Tests::runEvaluationTests();
	if (testResults.empty())
		return nullptr;

	return pEnv->NewStringUTF(testResults.c_str());
}

// region DebugStats

external JNIEXPORT jdouble JNICALL
Java_net_theluckycoder_chess_model_DebugStats_getNativeSearchTime(JNIEnv *, jclass)
{
	return static_cast<jdouble>(Stats::getElapsedMs());
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_model_DebugStats_getNativeBoardEvaluation(JNIEnv *, jclass)
{
	return static_cast<jint>(Evaluation::value(BoardManager::getBoard()));
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_model_DebugStats_getNativeAdvancedStats(JNIEnv *pEnv, jclass)
{
	const auto stats = Stats::formatStats('\n');
	return pEnv->NewStringUTF(stats.c_str());
}

// endregion DebugStats
