#pragma once

#include <jni.h>

// JVM Cached Classes
namespace JniCache
{
	jclass viewModelClass;
	jclass indexedPieceClass;
	jclass moveClass;
	jclass engineSettingsClass;

	static jclass cacheClass(JNIEnv *env, jclass cls)
	{
		return static_cast<jclass>(env->NewGlobalRef(cls));
	}

	void createCaches(JNIEnv *env)
	{
		viewModelClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/ChessViewModel"));
		indexedPieceClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/model/IndexedPiece"));
		moveClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/model/Move"));
		engineSettingsClass = cacheClass(env, env->FindClass("net/theluckycoder/chess/model/EngineSettings"));
	}

	void cleanCaches(JNIEnv *env)
	{
		env->DeleteGlobalRef(viewModelClass);
		env->DeleteGlobalRef(indexedPieceClass);
		env->DeleteGlobalRef(moveClass);
		env->DeleteGlobalRef(engineSettingsClass);
	}
}
