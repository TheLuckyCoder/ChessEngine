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

	template <typename ResultType, typename Func, typename... Args>
	static ThreadPool::TaskFuture<ResultType> submitJob(Func &&func, Args &&... args)
	{
		return pool.submit<ResultType>(std::forward<Func>(func), std::forward<Args>(args)...);
	}
};
