#pragma once

#include <jni.h>

// JVM Cached Classes
namespace Cache
{
	jclass gameManagerClass;
	jclass posClass;
	jclass posPairClass;
	jclass pieceClass;
	jclass moveClass;

	jclass cacheClass(JNIEnv *env, jclass cls)
	{
		return static_cast<jclass>(env->NewGlobalRef(cls));
	}

	void createCaches(JNIEnv *env)
	{
		gameManagerClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/GameManager"));
		posClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/model/Pos"));
		posPairClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/model/PosPair"));
		pieceClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/model/Piece"));
		moveClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/model/Move"));
	}

	void cleanCaches(JNIEnv *env)
	{
		env->DeleteGlobalRef(gameManagerClass);
		env->DeleteGlobalRef(posClass);
		env->DeleteGlobalRef(posPairClass);
		env->DeleteGlobalRef(pieceClass);
		env->DeleteGlobalRef(moveClass);
	}
}