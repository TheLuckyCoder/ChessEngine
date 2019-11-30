#pragma once

#include "ThreadSafeQueue.hpp"

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool
{
public:

	explicit ThreadPool(const std::size_t numThreads = std::thread::hardware_concurrency())
		: m_Done{ false }
	{
		m_Threads.reserve(numThreads);
		try
		{
			for (auto i = 0u; i < numThreads; ++i)
				m_Threads.emplace_back(&ThreadPool::worker, this);
			m_WaitingThreads = numThreads;
		}
		catch (...)
		{
			destroy();
			throw;
		}
	}

	/**
	 * Non-copyable.
	 */
	ThreadPool(const ThreadPool&) = delete;

	/**
	 * Non-assignable.
	 */
	ThreadPool& operator=(const ThreadPool&) = delete;

	/**
	 * Destructor.
	 */
	~ThreadPool()
	{
		destroy();
	}

	std::size_t threadCount() const noexcept
	{
		return m_Threads.size();
	}

	void updateThreadCount(const std::size_t numThreads) noexcept(false)
	{
		const std::size_t currentThreadCount = threadCount();

		if (numThreads < currentThreadCount)
		{
			// Destroy all the threads and invalidate the queue
			{
				m_Done = true;
				m_Queue.invalidate();
				for (auto &thread : m_Threads)
					if (thread.joinable())
						thread.join();

				m_Threads.clear();
			}

			{
				m_Threads.reserve(numThreads);
				m_Done = false;

				m_Queue.~ThreadSafeQueue();
				new(&m_Queue) QueueType();

				for (auto i = 0u; i < numThreads; ++i)
					m_Threads.emplace_back(&ThreadPool::worker, this);
				
				m_WaitingThreads = numThreads;
			}
		} else if (numThreads > currentThreadCount) {
			// Add the extra number of threads necessary
			m_Threads.reserve(numThreads);

			for (auto i = currentThreadCount; i < numThreads; ++i)
				m_Threads.emplace_back(&ThreadPool::worker, this);

			m_WaitingThreads = numThreads;
		}
		

		// Otherwise the size should be the same
	}


	template <typename Func, typename... Args>
	void submitWork(Func &&func, Args &&...args)
	{
		auto work = [func, args...] { func(args...); };
		m_Queue.push(work);
	}

	template <typename Func, typename... Args>
	auto submitTask(Func &&func, Args &&...args) -> std::future<std::invoke_result_t<Func, Args...>>
	{
		using ReturnType = std::invoke_result_t<Func, Args...>;
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<Func>(func),
			std::forward<Args>(args)...)
		);
		std::future<ReturnType> result = task->get_future();

		auto work = [task] { (*task)(); };
		m_Queue.push(work);

		return result;
	}

private:
	/**
	 * Constantly running function each thread uses to acquire work items from the queue.
	 */
	void worker()
	{
		while (!m_Done)
		{
			Proc func;
			if (m_Queue.waitPop(func))
			{
				--m_WaitingThreads;
				func();
				if (++m_WaitingThreads == threadCount())
					m_JoinCondition.notify_one();
			}
		}
	}

	/**
	 * Invalidates the queue and joins all running threads.
	 */
	void destroy()
	{
		m_Done = true;
		m_Queue.invalidate();
		for (auto &thread : m_Threads)
			if (thread.joinable())
				thread.join();
	}

private:
	using Proc = std::function<void(void)>;
	using QueueType = ThreadSafeQueue<Proc>;

	std::atomic_bool m_Done;
	QueueType m_Queue;
	std::vector<std::thread> m_Threads;
	std::atomic_size_t m_WaitingThreads;
	std::condition_variable m_JoinCondition;
};
