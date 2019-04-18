#pragma once

#include <jni.h>

// JVM Cached Classes
namespace Cache
{
	jclass chessActivityClass;
	jclass posClass;
	jclass posPairClass;
	jclass pieceClass;

	jclass cacheClass(JNIEnv *env, jclass cls)
	{
		return static_cast<jclass>(env->NewGlobalRef(cls));
	}

	void createCaches(JNIEnv *env)
	{
		posClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/Pos"));
		posPairClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/PosPair"));
		pieceClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/Piece"));
	}

	void cleanCaches(JNIEnv *env)
	{
		env->DeleteGlobalRef(chessActivityClass);
		env->DeleteGlobalRef(posClass);
		env->DeleteGlobalRef(posPairClass);
		env->DeleteGlobalRef(pieceClass);
	}
}