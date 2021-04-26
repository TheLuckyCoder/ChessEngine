#pragma once

#include <jni.h>

// JVM Cached Classes
namespace JniCache
{
	jclass boardChangeListenerClass;
	jclass searchListenerClass;
	jclass indexedPieceClass;
	jclass moveClass;
	jclass searchOptionsClass;

	static jclass cacheClass(JNIEnv *env, const char *name)
	{
		return static_cast<jclass>(env->NewGlobalRef(env->FindClass(name)));
	}

	void createCaches(JNIEnv *env)
	{
		boardChangeListenerClass = cacheClass(env, "net/theluckycoder/chess/cpp/BoardChangeListener");
		searchListenerClass = cacheClass(env, "net/theluckycoder/chess/cpp/SearchListener");
		indexedPieceClass = cacheClass(env, "net/theluckycoder/chess/model/IndexedPiece");
		moveClass = cacheClass(env, "net/theluckycoder/chess/model/Move");
		searchOptionsClass = cacheClass(env, "net/theluckycoder/chess/model/SearchOptions");
	}

	void cleanCaches(JNIEnv *env)
	{
		env->DeleteGlobalRef(boardChangeListenerClass);
		env->DeleteGlobalRef(searchListenerClass);
		env->DeleteGlobalRef(indexedPieceClass);
		env->DeleteGlobalRef(moveClass);
		env->DeleteGlobalRef(searchOptionsClass);
	}
}
