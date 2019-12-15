#pragma once

#define external extern "C"

int getInt(JNIEnv *env, jclass type, jobject obj, const char *name)
{
	return static_cast<int>(env->GetIntField(obj, env->GetFieldID(type, name, "I")));
}
