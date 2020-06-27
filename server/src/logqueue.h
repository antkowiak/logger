#pragma once

//
// logqueue.h - Synchronized queue of log entries to send to the log server.
//
// Written by Ryan Antkowiak (antkowiak@gmail.com)
//

#include <mutex>
#include <queue>

#include "logentry.h"

// queue of log entries
class logqueue
{
private:
	// mutex to protect access to the queue
	mutable std::mutex m_mutex;

	// the queue of log entries
	std::queue<logentry> m_queue;

public:
	// push a log entry into the queue
	void push(const logentry& entry)
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		m_queue.push(entry);
	}

	// return true if the queue is empty
	bool empty() const
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		return m_queue.empty();
	}

	// pops all entries out of the queue, places them into a vector, and returns the vector of entries.
	std::vector<logentry> pop()
	{
		std::vector<logentry> entries;

		{
			std::lock_guard<std::mutex> guard(m_mutex);

			entries.reserve(m_queue.size());

			while (!m_queue.empty())
			{
				entries.emplace_back(m_queue.front());
				m_queue.pop();
			}
		}

		return entries;
	}
};
