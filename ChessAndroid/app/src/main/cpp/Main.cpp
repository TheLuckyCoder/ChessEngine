#include <jni.h>

#include "Log.h"
#include "external.h"

#include "chess/BoardManager.h"
#include "chess/persistence/JsonPersistence.h"
#include "chess/persistence/MovesPersistence.h"
#include "chess/Utils.h"

JavaVM *jvm;

bool boardManagerInitialized = false;

jobject mainActivityInstance;
jclass mainActivityClass;
jclass posClass;
jclass posPairClass;
jclass pieceClass;

const BoardManager::PieceChangeListener listener = [](GameState state, bool shouldRedraw, const StackVector<PosPair, 2> &moved) {
	JNIEnv *env;
	int getEnvStat = jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->AttachCurrentThread(&env, NULL);
		LOGI("ChessCpp", "Attached to Thread");
	}

	env->ExceptionClear();

	jobjectArray result = env->NewObjectArray(static_cast<jsize>(moved.size()), posPairClass, NULL);
	{
		const static jmethodID constructorId = env->GetMethodID(posPairClass, "<init>", "(SSSS)V");

		for (unsigned i = 0; i < moved.size(); ++i) {
			auto &pair = moved[i];
			jobject obj = env->NewObject(posPairClass, constructorId,
										 pair.first.x, pair.first.y, pair.second.x, pair.second.y);

			env->SetObjectArrayElement(result, i, obj);
		}
	}

	const static jmethodID callbackId = env->GetMethodID(mainActivityClass, "callback", "(IZ[Lnet/theluckycoder/chess/PosPair;)V");
	env->CallVoidMethod(mainActivityInstance, callbackId, static_cast<jint>(to_underlying(state)), shouldRedraw, result);

	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->DetachCurrentThread();
		LOGI("ChessCpp", "Detached from Thread");
	}
};

external JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void __unused *reserved)
{
    LOGI("ChessCpp", "JNI_OnLoad");

    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    jvm = vm;

    posClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/Pos"));
    posPairClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/PosPair"));
    pieceClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/Piece"));

    return JNI_VERSION_1_6;
}

external JNIEXPORT void JNI_OnUnload(JavaVM *vm, void __unused *reserved)
{
    LOGI("ChessCpp", "JNI_OnUnload");

    JNIEnv* env;
    vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

    // Cleaning the caches
    env->DeleteGlobalRef(mainActivityInstance);
    env->DeleteGlobalRef(mainActivityClass);

    env->DeleteGlobalRef(posClass);
    env->DeleteGlobalRef(posPairClass);
    env->DeleteGlobalRef(pieceClass);
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_MainActivity_initBoard(JNIEnv *pEnv, jobject instance, jboolean restartGame)
{
	pEnv->ExceptionClear();
	if (!boardManagerInitialized)
	{
		pEnv->DeleteGlobalRef(mainActivityInstance);
		mainActivityInstance = pEnv->NewGlobalRef(instance);
		mainActivityClass = cacheClass(pEnv, pEnv->GetObjectClass(mainActivityInstance));

		boardManagerInitialized = true;
		BoardManager::initBoardManager(listener);
	}

	if (restartGame)
		BoardManager::initBoardManager(listener);
}

// Native Class
external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_isPlayerWhite(JNIEnv  __unused *pEnv, __unused jclass type)
{
    return static_cast<jboolean>(BoardManager::isPlayerWhite);
}

external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_isWorking(JNIEnv __unused *pEnv, jclass __unused type)
{
    return static_cast<jboolean>(BoardManager::isWorking());
}

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getNumberOfEvaluatedBoards(JNIEnv __unused *pEnv, jclass __unused type)
{
	return static_cast<jint>(BoardManager::boardsEvaluated);
}

external JNIEXPORT _jobjectArray* JNICALL
Java_net_theluckycoder_chess_Native_getPieces(JNIEnv *pEnv, jclass __unused type)
{
    pEnv->ExceptionClear();

    const static jmethodID constructorId = pEnv->GetMethodID(pieceClass, "<init>", "(SSB)V");

    const auto pieces = BoardManager::getBoard().getAllPieces();
    auto *array = pEnv->NewObjectArray(static_cast<jsize>(pieces.size()), pieceClass, NULL);

    jsize i = 0;
    for (const auto &it: pieces) {
        const Pos &pos = it.first;
        jbyte pieceType = static_cast<jbyte>(to_underlying(it.second.type));
        if (!it.second.isWhite)
            pieceType += 6;

        jobject obj = pEnv->NewObject(pieceClass, constructorId, pos.x, pos.y, pieceType);
        pEnv->SetObjectArrayElement(array, i, obj);

        ++i;
    }

    return array;
}

external JNIEXPORT _jobjectArray* JNICALL
Java_net_theluckycoder_chess_Native_getPossibleMoves(JNIEnv *pEnv, jclass __unused type, jobject dest)
{
    pEnv->ExceptionClear();

	const static jmethodID constructorId = pEnv->GetMethodID(posClass, "<init>", "(SS)V");

    const Pos pos(getByte(pEnv, posClass, dest, "x"), getByte(pEnv, posClass, dest, "y"));
    const auto possibleMoves = BoardManager::getPossibleMoves(pos);
    auto *result = pEnv->NewObjectArray(static_cast<jsize>(possibleMoves.size()), posClass, NULL);

    for (unsigned i = 0; i < possibleMoves.size(); ++i) {
        jobject obj = pEnv->NewObject(posClass, constructorId, possibleMoves[i].x, possibleMoves[i].y);
        pEnv->SetObjectArrayElement(result, i, obj);
    }

    return result;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_movePiece(JNIEnv __unused *pEnv, __unused jclass type,
											  jshort selectedX, jshort selectedY, jshort destX, jshort destY)
{
    BoardManager::movePiece(Pos(static_cast<byte>(selectedX), static_cast<byte>(selectedY)),
							Pos(static_cast<byte>(destX), static_cast<byte>(destY)));
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_loadFromJson(JNIEnv __unused *pEnv, __unused jclass type, jstring json)
{
	const char *nativeString = pEnv->GetStringUTFChars(json, 0);

	BoardManager::loadGame(JsonPersistence::load(nativeString));

	pEnv->ReleaseStringUTFChars(json, nativeString);
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_saveToJson(JNIEnv __unused *pEnv, __unused jclass type)
{
	return pEnv->NewStringUTF(JsonPersistence::save(BoardManager::getBoard()).c_str());
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_loadMoves(JNIEnv __unused *pEnv, __unused jclass type, jstring moves)
{
	const char *nativeString = pEnv->GetStringUTFChars(moves, 0);

	BoardManager::loadGame(MovesPersistence::load(nativeString));

	pEnv->ReleaseStringUTFChars(moves, nativeString);
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_saveMoves(JNIEnv __unused *pEnv, __unused jclass type)
{
	return pEnv->NewStringUTF(MovesPersistence::save(BoardManager::getMovesHistory()).c_str());
}