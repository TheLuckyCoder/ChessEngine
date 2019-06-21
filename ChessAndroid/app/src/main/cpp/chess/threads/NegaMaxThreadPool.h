#pragma once

#include "ThreadPool.hpp"

class NegaMaxThreadPool final
{
	inline static ThreadPool pool{ 1 };

public:
	NegaMaxThreadPool() = delete;

	static void updateThreadCount(const unsigned int threadCount)
	{
		if (pool.threadCount() != threadCount)
		{
			try
			{
				pool.updateThreadCount(threadCount);
			} catch (...)
			{
			}
		}
	}

	template <typename Func, typename... Args>
	static ThreadPool::TaskFuture<void> submitJob(Func &&func, Args &&... args)
	{
		return pool.submit<void>(std::forward<Func>(func), std::forward<Args>(args)...);
	}
};
