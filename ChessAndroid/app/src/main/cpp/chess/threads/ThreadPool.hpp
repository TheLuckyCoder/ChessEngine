#pragma once

#include "ThreadSafeQueue.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool
{
private:
	class IThreadTask
	{
	public:
		IThreadTask() = default;
		virtual ~IThreadTask() = default;
		IThreadTask(const IThreadTask&) = delete;
		IThreadTask &operator=(const IThreadTask&) = delete;
		IThreadTask(IThreadTask&&) noexcept = default;
		IThreadTask &operator=(IThreadTask&&) noexcept = default;

		/**
		 * Run the task.
		 */
		virtual void execute() = 0;
	};

	template <typename Func>
	class ThreadTask : public IThreadTask
	{
	public:
		explicit ThreadTask(Func &&func)
			: m_func{ std::move(func) } {}

		~ThreadTask() override = default;
		ThreadTask(const ThreadTask &rhs) = delete;
		ThreadTask &operator=(const ThreadTask &rhs) = delete;
		ThreadTask(ThreadTask&&) noexcept = default;
		ThreadTask &operator=(ThreadTask&&) noexcept = default;

		/**
		 * Run the task.
		 */
		void execute() override
		{
			m_func();
		}

	private:
		Func m_func;
	};

public:
	/**
	 * A wrapper around a std::future that adds the behavior of futures returned from std::async.
	 * Specifically, this object will block and wait for execution to finish before going out of scope.
	 */
	template <typename T>
	class TaskFuture
	{
	public:
		explicit TaskFuture(std::future<T> &&future)
			: m_future{ std::move(future) } {}

		TaskFuture(const TaskFuture &rhs) = delete;
		TaskFuture &operator=(const TaskFuture &rhs) = delete;
		TaskFuture(TaskFuture &&other) = default;
		TaskFuture &operator=(TaskFuture &&other) = default;
		~TaskFuture()
		{
			if (m_future.valid())
				m_future.get();
		}

		auto get()
		{
			return m_future.get();
		}

		bool ready(const std::size_t millis) const
		{
			return m_future.wait_for(std::chrono::milliseconds(millis)) == std::future_status::ready;
		}


	private:
		std::future<T> m_future;
	};

public:
	/**
	 * Constructor.
	 */
	ThreadPool()
		: ThreadPool { std::max(std::thread::hardware_concurrency() - 1u, 2u) }
	{
		/*
		 * Always create at least two threads.
		 */
	}

	/**
	 * Constructor.
	 */
	explicit ThreadPool(const std::size_t numThreads)
		: m_done{ false }
	{
		m_threads.reserve(numThreads);
		try
		{
			for (auto i = 0u; i < numThreads; ++i)
				m_threads.emplace_back(&ThreadPool::worker, this);
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
		return m_threads.size();
	}

	void updateThreadCount(const std::size_t numThreads) noexcept(false)
	{
		const std::size_t currentThreadCount = threadCount();

		if (numThreads < currentThreadCount)
		{
			m_done = true;
			for (auto &thread : m_threads)
				if (thread.joinable())
					thread.join();

			m_threads.reserve(numThreads);
			m_threads.clear();

			try
			{
				for (auto i = 0u; i < numThreads; ++i)
					m_threads.emplace_back(&ThreadPool::worker, this);
			}
			catch (...)
			{
				destroy();
				throw;
			}

			m_done = false;
		} else if (numThreads > currentThreadCount) {
			m_threads.reserve(numThreads);

			for (auto i = currentThreadCount; i < numThreads; ++i)
				m_threads.emplace_back(&ThreadPool::worker, this);
		}

		// Else the size should be the same
	}

	/**
	 * Submit a job to be run by the thread pool.
	 */
	template <typename ResultType, typename Func, typename... Args>
	TaskFuture<ResultType> submit(Func &&func, Args &&...args)
	{
		auto boundTask = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
		using PackagedTask = std::packaged_task<ResultType()>;
		using TaskType = ThreadTask<PackagedTask>;

		PackagedTask task{ std::move(boundTask) };
		TaskFuture<ResultType> result{ task.get_future() };
		m_workQueue.push(std::make_unique<TaskType>(std::move(task)));
		return result;
	}

private:
	/**
	 * Constantly running function each thread uses to acquire work items from the queue.
	 */
	void worker()
	{
		while (!m_done)
		{
			std::unique_ptr<IThreadTask> pTask{ nullptr };
			if (m_workQueue.waitPop(pTask))
				pTask->execute();
		}
	}

	/**
	 * Invalidates the queue and joins all running threads.
	 */
	void destroy()
	{
		m_done = true;
		m_workQueue.invalidate();
		for (auto &thread : m_threads)
			if (thread.joinable())
				thread.join();
	}

private:
	std::atomic_bool m_done;
	ThreadSafeQueue<std::unique_ptr<IThreadTask>> m_workQueue;
	std::vector<std::thread> m_threads;
};
