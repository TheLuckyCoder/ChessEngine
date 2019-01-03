#pragma once

#define external extern "C"

using byte = unsigned char;

byte getByte(JNIEnv *env, jclass type, jobject obj, const char *name) {
    return static_cast<byte>(env->GetShortField(obj, env->GetFieldID(type, name, "S")));
}

jclass cacheClass(JNIEnv *env, jclass cls) {
    return static_cast<jclass>(env->NewGlobalRef(cls));
}
