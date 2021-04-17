#pragma once

#include <jni.h>

// JVM Cached Classes
namespace JniCache
{
	jclass boardChangeListenerClass;
	jclass indexedPieceClass;
	jclass moveClass;
	jclass engineSettingsClass;

	static jclass cacheClass(JNIEnv *env, const char *name)
	{
		return static_cast<jclass>(env->NewGlobalRef(env->FindClass(name)));
	}

	void createCaches(JNIEnv *env)
	{
		boardChangeListenerClass = cacheClass(env, "net/theluckycoder/chess/BoardChangeListener");
		indexedPieceClass = cacheClass(env, "net/theluckycoder/chess/model/IndexedPiece");
		moveClass = cacheClass(env, "net/theluckycoder/chess/model/Move");
		engineSettingsClass = cacheClass(env, "net/theluckycoder/chess/model/EngineSettings");
	}

	void cleanCaches(JNIEnv *env)
	{
		env->DeleteGlobalRef(boardChangeListenerClass);
		env->DeleteGlobalRef(indexedPieceClass);
		env->DeleteGlobalRef(moveClass);
		env->DeleteGlobalRef(engineSettingsClass);
	}
}
