#include <jni.h>

#include "Log.h"
#include "external.h"

#include "chess/BoardManager.h"
#include "chess/data/Board.h"
#include "chess/minimax/Evaluation.h"
#include "chess/persistence/MovesPersistence.h"
#include "chess/Utils.h"

JavaVM *jvm;

bool boardManagerInitialized = false;

jobject mainActivityInstance;
jclass mainActivityClass;
jclass posClass;
jclass posPairClass;
jclass pieceClass;

const BoardManager::PieceChangeListener listener = [](State state, bool shouldRedraw, const StackVector<PosPair, 2> &moved) {
	JNIEnv *env;
	int getEnvStat = jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
	{
		jvm->AttachCurrentThread(&env, nullptr);
		LOGI("ChessCpp", "Attached to Thread");
	}

	env->ExceptionClear();

	jobjectArray result = env->NewObjectArray(static_cast<jsize>(moved.size()), posPairClass, nullptr);
	{
		const static jmethodID constructorId = env->GetMethodID(posPairClass, "<init>", "(BBBB)V");

		for (unsigned i = 0; i < moved.size(); ++i)
		{
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
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
        return -1;

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

external JNIEXPORT jint JNICALL
Java_net_theluckycoder_chess_Native_getCurrentBoardEvaluation(JNIEnv __unused *pEnv, jclass __unused type)
{
	return static_cast<jint>(Evaluation::evaluate(BoardManager::getBoard()));
}

external JNIEXPORT _jobjectArray* JNICALL
Java_net_theluckycoder_chess_Native_getPieces(JNIEnv *pEnv, jclass __unused type)
{
    pEnv->ExceptionClear();

    const static jmethodID constructorId = pEnv->GetMethodID(pieceClass, "<init>", "(BBB)V");

    const auto pieces = BoardManager::getBoard().getAllPieces();
    auto *array = pEnv->NewObjectArray(static_cast<jsize>(pieces.size()), pieceClass, nullptr);

    jsize i = 0;
    for (const auto &it : pieces)
    {
        const Pos &pos = it.first;
        auto pieceType = static_cast<jbyte>(to_underlying(it.second.type));
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

	const static jmethodID constructorId = pEnv->GetMethodID(posClass, "<init>", "(BB)V");

    const Pos pos(getByte(pEnv, posClass, dest, "x"), getByte(pEnv, posClass, dest, "y"));
    const auto possibleMoves = BoardManager::getPossibleMoves(pos);
    auto *result = pEnv->NewObjectArray(static_cast<jsize>(possibleMoves.size()), posClass, nullptr);

    for (unsigned i = 0; i < possibleMoves.size(); ++i)
    {
        jobject obj = pEnv->NewObject(posClass, constructorId, possibleMoves[i].x, possibleMoves[i].y);
        pEnv->SetObjectArrayElement(result, i, obj);
    }

    return result;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_movePiece(JNIEnv __unused *pEnv, __unused jclass type,
											  jbyte selectedX, jbyte selectedY, jbyte destX, jbyte destY)
{
    BoardManager::movePiece(Pos(static_cast<byte>(selectedX), static_cast<byte>(selectedY)),
							Pos(static_cast<byte>(destX), static_cast<byte>(destY)));
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_loadMoves(JNIEnv __unused *pEnv, __unused jclass type, jstring moves)
{
	const char *nativeString = pEnv->GetStringUTFChars(moves, nullptr);

	BoardManager::loadGame(MovesPersistence::load(nativeString));

	pEnv->ReleaseStringUTFChars(moves, nativeString);
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_saveMoves(JNIEnv __unused *pEnv, __unused jclass type)
{
	return pEnv->NewStringUTF(MovesPersistence::save(BoardManager::getMovesHistory()).c_str());
}
