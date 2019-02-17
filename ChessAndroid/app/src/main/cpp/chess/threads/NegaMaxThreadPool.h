#pragma once

#include "ThreadPool.hpp"

class NegaMaxThreadPool final
{
    inline static ThreadPool *pool = nullptr;

public:
    NegaMaxThreadPool() = delete;

    static void createThreadPool(unsigned int threadCount)
    {
        if (!pool)
            pool = new ThreadPool(threadCount);
        else if (pool->threadCount() != threadCount)
        {
            pool->~ThreadPool();
            new (pool) ThreadPool(threadCount);
        }
    }

    /**
     * Submit a job to the default thread pool.
     */
    template <typename ResultType, typename Func, typename... Args>
    static ThreadPool::TaskFuture<ResultType> submitJob(Func&& func, Args&&... args)
    {
        return pool->submit<ResultType>(std::forward<Func>(func), std::forward<Args>(args)...);
    }
};
