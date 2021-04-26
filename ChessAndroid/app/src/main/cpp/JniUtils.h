#pragma once

#include <android/log.h>
#include <cassert>

#define LOGV(LOG_TAG, ...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGI(LOG_TAG, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(LOG_TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGW(LOG_TAG, ...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(LOG_TAG, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

class NativeThreadAttach
{
public:
	explicit NativeThreadAttach(JavaVM *jvm)
		: _jvm(jvm)
	{
		_jvmEnvState = _jvm->GetEnv(reinterpret_cast<void **>(&_env), JNI_VERSION_1_6);

		if (_jvmEnvState == JNI_EDETACHED)
			_jvm->AttachCurrentThread(&_env, nullptr);
		_env->ExceptionClear();
	}

	JNIEnv *getEnv() { return _env; }

	~NativeThreadAttach()
	{
		if (_jvmEnvState == JNI_EDETACHED)
			_jvm->DetachCurrentThread();
	}

private:
	JavaVM *_jvm;
	JNIEnv *_env = nullptr;
	int _jvmEnvState{};
};
