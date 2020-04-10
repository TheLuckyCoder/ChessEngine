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
		std::lock_guard lock(_mutex);
		if (_queue.empty() || !_valid)
		{
			return false;
		}
		out = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	/**
	 * Get the first value in the queue.
	 * Will block until a value is available unless clear is called or the instance is destructed.
	 * Returns true if a value was successfully written to the out parameter, false otherwise.
	 */
	bool waitPop(T &out)
	{
		std::unique_lock lock(_mutex);
		_condition.wait(lock, [this]()
		{
			return !_queue.empty() || !_valid;
		});
		/*
		 * Using the condition in the predicate ensures that spurious wakeups with a valid
		 * but empty queue will not proceed, so only need to check for validity before proceeding.
		 */
		if (!_valid)
			return false;
		
		out = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	void push(const T &value)
	{
		std::lock_guard lock(_mutex);
		_queue.push(value);
		_condition.notify_one();
	}
	
	void push(T &&value)
	{
		std::lock_guard lock(_mutex);
		_queue.push(std::move(value));
		_condition.notify_one();
	}

	bool empty() const
	{
		std::lock_guard lock(_mutex);
		return _queue.empty();
	}

	void clear()
	{
		std::lock_guard lock(_mutex);
		while (!_queue.empty())
		{
			_queue.pop();
		}
		_condition.notify_all();
	}

	/**
	 * Invalidate the queue.
	 * Used to ensure no conditions are being waited on in waitPop when
	 * a thread or the application is trying to exit.
	 */
	void invalidate()
	{
		std::lock_guard lock(_mutex);
		_valid = false;
		_condition.notify_all();
	}

	bool isValid() const
	{
		std::lock_guard lock(_mutex);
		return _valid;
	}

private:
	std::atomic_bool _valid = true;
	mutable std::mutex _mutex;
	std::queue<T> _queue;
	std::condition_variable _condition;
};
