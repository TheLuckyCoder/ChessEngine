#pragma once

#include "ThreadSafeQueue.h"

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
	{
		_threads.reserve(numThreads);
		try
		{
			for (auto i = 0u; i < numThreads; ++i)
				_threads.emplace_back(&ThreadPool::worker, this);
		}
		catch (...)
		{
			destroy();
			throw;
		}
	}

	ThreadPool(const ThreadPool&) = delete;

	ThreadPool& operator=(const ThreadPool&) = delete;

	~ThreadPool()
	{
		destroy();
	}

	std::size_t getThreadCount() const noexcept
	{
		return _threads.size();
	}

	void updateThreadCount(const std::size_t numThreads) noexcept(false)
	{
		const std::size_t currentThreadCount = getThreadCount();

		if (numThreads < currentThreadCount)
		{
			// Destroy all the threads and invalidate the queue
			{
				_done = true;
				_queue.invalidate();
				for (auto &thread : _threads)
					if (thread.joinable())
						thread.join();

				_threads.clear();
			}

			{
				_threads.reserve(numThreads);
				_done = false;

				_queue.~ThreadSafeQueue();
				new(&_queue) QueueType();

				for (auto i = 0u; i < numThreads; ++i)
					_threads.emplace_back(&ThreadPool::worker, this);
			}
		} else if (numThreads > currentThreadCount) {
			// Add the extra number of threads necessary
			_threads.reserve(numThreads);

			for (auto i = currentThreadCount; i < numThreads; ++i)
				_threads.emplace_back(&ThreadPool::worker, this);
		}
		

		// Otherwise the size should be the same
	}

	template <typename Func, typename... Args>
	void submitWork(Func &&func, Args &&...args)
	{
		auto work = [func, args...] { func(args...); };
		_queue.push(work);
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
		_queue.push(work);

		return result;
	}

private:
	/**
	 * Constantly running function each thread uses to acquire work items from the queue.
	 */
	void worker()
	{
		while (!_done)
		{
			Proc func;
			if (_queue.waitPop(func))
				func();
		}
	}

	/**
	 * Invalidates the queue and joins all running threads.
	 */
	void destroy()
	{
		_done = true;
		_queue.invalidate();
		for (auto &thread : _threads)
			if (thread.joinable())
				thread.join();
	}

private:
	using Proc = std::function<void(void)>;
	using QueueType = ThreadSafeQueue<Proc>;

	std::atomic_bool _done = false;
	QueueType _queue;
	std::vector<std::thread> _threads;
};
