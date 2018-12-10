#pragma once

#include <type_traits>

#define external extern "C"

short getShort(JNIEnv *env, jclass type, jobject obj, const char *name) {
    return env->GetShortField(obj, env->GetFieldID(type, name, "S"));
}

jclass cacheClass(JNIEnv *env, jclass cls) {
    return static_cast<jclass>(env->NewGlobalRef(cls));
}

template<typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
