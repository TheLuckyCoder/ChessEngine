#include <jni.h>

#include "Log.h"
#include "external.h"

#include "chess/BoardManager.h"
#include "chess/SaveLoadJson.h"
#include "chess/Utils.h"

JavaVM *jvm;

jobject mainActivityInstance;
jclass mainActivityClass;
jclass posPairClass;

const BoardManager::PieceChangeListener listener = [](GameState state, bool shouldRedraw, const std::vector<PosPair> &moved) {
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

	const static jmethodID callbackId = env->GetMethodID(mainActivityClass, "callback", "(BZ[Lnet/theluckycoder/chess/PosPair;)V");
	env->CallVoidMethod(mainActivityInstance, callbackId, static_cast<jbyte>(to_underlying(state)), shouldRedraw, result);

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

    posPairClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/PosPair"));

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
    env->DeleteGlobalRef(posPairClass);
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_MainActivity_initBoard(JNIEnv *pEnv, jobject instance)
{
    pEnv->ExceptionClear();
    mainActivityInstance = pEnv->NewGlobalRef(instance);
    mainActivityClass = cacheClass(pEnv, pEnv->GetObjectClass(mainActivityInstance));

    BoardManager::initBoardManager(listener);
}

// Native Class
external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_isWhiteAtBottom(JNIEnv  __unused *pEnv, __unused jclass type)
{
    return static_cast<jboolean>(BoardManager::isWhiteAtBottom);
}

external JNIEXPORT jboolean JNICALL
Java_net_theluckycoder_chess_Native_isWorking(JNIEnv __unused *pEnv, jclass __unused type)
{
    return static_cast<jboolean>(BoardManager::isWorking());
}

external JNIEXPORT _jobjectArray* JNICALL
Java_net_theluckycoder_chess_Native_getPieces(JNIEnv *pEnv, jclass __unused type)
{
    pEnv->ExceptionClear();

    jclass pieceClass = pEnv->FindClass("net/theluckycoder/chess/Piece");
    jmethodID constructorId = pEnv->GetMethodID(pieceClass, "<init>", "(SSB)V");

    auto pieces = BoardManager::getBoard().getAllPieces();
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

    jclass cls = pEnv->FindClass("net/theluckycoder/chess/Pos");
    jmethodID constructorId = pEnv->GetMethodID(cls, "<init>", "(SS)V");

    Pos pos = Pos(getShort(pEnv, cls, dest, "x"), getShort(pEnv, cls, dest, "y"));

    Piece &piece = BoardManager::getBoard()[pos];
    if (!piece)
    {
        LOGE("ChessCpp", "The Selected Piece is empty");
        return nullptr;
    }

    auto possibleMoves = piece.getPossibleMoves(pos, BoardManager::getBoard());
    auto *result = pEnv->NewObjectArray(static_cast<jsize>(possibleMoves.size()), cls, NULL);

    for (unsigned i = 0; i < possibleMoves.size(); ++i) {
        jobject obj = pEnv->NewObject(cls, constructorId, possibleMoves[i].x, possibleMoves[i].y);
        pEnv->SetObjectArrayElement(result, i, obj);
    }

    return result;
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_movePiece(JNIEnv __unused *pEnv, __unused jclass type, jshort selectedX,
                                              jshort selectedY, jshort destX, jshort destY)
{
    Pos selected(selectedX, selectedY);
    Pos dest(destX, destY);
    BoardManager::movePiece(selected, dest);
}

external JNIEXPORT void JNICALL
Java_net_theluckycoder_chess_Native_loadFromJson(JNIEnv __unused *pEnv, __unused jclass type, jstring json)
{
	const char *nativeString = pEnv->GetStringUTFChars(json, 0);

	BoardManager::loadJsonGame(SaveLoadJson::load(nativeString));

	pEnv->ReleaseStringUTFChars(json, nativeString);
}

external JNIEXPORT jstring JNICALL
Java_net_theluckycoder_chess_Native_saveToJson(JNIEnv __unused *pEnv, __unused jclass type)
{
	const auto json = SaveLoadJson::save(BoardManager::getBoard());
	return pEnv->NewStringUTF(json.c_str());
}
