#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

template <typename T>
class ThreadSafeQueue
{
public:
	~ThreadSafeQueue()
	{
		invalidate();
	}

	/**
		* Attempt to get the first value in the queue.
		* Returns true if a value was successfully written to the out parameter, false otherwise.
		*/
	bool tryPop(T &out)
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		if (m_Queue.empty() || !m_Valid)
		{
			return false;
		}
		out = std::move(m_Queue.front());
		m_Queue.pop();
		return true;
	}

	/**
	 * Get the first value in the queue.
	 * Will block until a value is available unless clear is called or the instance is destructed.
	 * Returns true if a value was successfully written to the out parameter, false otherwise.
	 */
	bool waitPop(T &out)
	{
		std::unique_lock<std::mutex> lock{ m_Mutex };
		m_Condition.wait(lock, [this]()
		{
			return !m_Queue.empty() || !m_Valid;
		});
		/*
		 * Using the condition in the predicate ensures that spurious wakeups with a valid
		 * but empty queue will not proceed, so only need to check for validity before proceeding.
		 */
		if (!m_Valid)
			return false;
		
		out = std::move(m_Queue.front());
		m_Queue.pop();
		return true;
	}

	void push(const T &value)
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		m_Queue.push(value);
		m_Condition.notify_one();
	}
	
	void push(T &&value)
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		m_Queue.push(std::move(value));
		m_Condition.notify_one();
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		return m_Queue.empty();
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		while (!m_Queue.empty())
		{
			m_Queue.pop();
		}
		m_Condition.notify_all();
	}

	/**
		* Invalidate the queue.
		* Used to ensure no conditions are being waited on in waitPop when
		* a thread or the application is trying to exit.
		* The queue is invalid after calling this method and it is an error
		* to continue using a queue after this method has been called.
		*/
	void invalidate()
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		m_Valid = false;
		m_Condition.notify_all();
	}

	bool isValid() const
	{
		std::lock_guard<std::mutex> lock{ m_Mutex };
		return m_Valid;
	}

private:
	std::atomic_bool m_Valid{ true };
	mutable std::mutex m_Mutex;
	std::queue<T> m_Queue;
	std::condition_variable m_Condition;
};
