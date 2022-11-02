#pragma once

//
// logger.h - Singleton to buffer/queue up log entries for publishing to a log server.
//
// Written by Ryan Antkowiak 
//

#define _CRT_SECURE_NO_DEPRECATE
#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>

#pragma warning( push )
#pragma warning( disable : 6001 )
#pragma warning( disable : 6255 )
#pragma warning( disable : 6258 )
#pragma warning( disable : 6319 )
#pragma warning( disable : 26439 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 26495 )
#pragma warning( disable : 26498 )
#pragma warning( disable : 26812 )

#include <boost/bind.hpp>
#include <boost/asio.hpp>

// logger class
class logger
{
private:
	// singleton -- don't allow copy/move
	logger(const logger&) = delete;
	logger(logger&&) noexcept = delete;
	logger& operator = (const logger&) = delete;
	logger& operator = (logger&&) noexcept = delete;

	// constructor
	logger()
	{
		// start the publisher thread
		m_publisher = std::thread(&publisher_thread, std::ref(m_mutex), std::ref(m_queue));
	}

	// destructor
	~logger()
	{
		m_publisher.join();
	}

private:

	// the hostname where the log server is running
	constexpr static const char * const HOST = "localhost";

	// the port number the log server is listening on
	constexpr static const short PORT { 8080 };

	// mutex for accessing the queue
	std::mutex m_mutex;

	// queue of log messages to send to the server
	std::queue<std::string> m_queue;

	// publisher thread that reads messages out of the queue and sends to the server
	std::thread m_publisher;

private:

	// attempt to connect (reconnect) to the log server
	static void attempt_connection(boost::asio::ip::tcp::socket& socket, boost::asio::ip::tcp::resolver::iterator& endpoint)
	{
		bool connected = false;

		while (!connected)
		{
			try
			{
				boost::asio::connect(socket, endpoint);
				connected = true;
			}
			catch (...)
			{
				// five second delay in-between connection attempts
				std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			}
		}
	}

	// thread to pull log entries out of the (synchronized) queue and send to the server
	static void publisher_thread(std::mutex & mut, std::queue<std::string> & q)
	{
		// set up boost asio endpoint
		boost::asio::io_service svc;
		boost::asio::ip::tcp::socket socket(svc);
		boost::asio::ip::tcp::resolver resolver(svc);
		boost::asio::ip::tcp::resolver::iterator endpoint = resolver.resolve(boost::asio::ip::tcp::resolver::query(HOST, std::to_string(PORT)));

		// delay 1 sec to ensure endpoint is set up
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		// attempt to connect to log server
		attempt_connection(socket, endpoint);

		// loop forever
		while (true)
		{
			// only peek at the log queue every 100ms
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			
			// vector to hold a local copy of log entries that will be popped out of the synchornied queue
			std::vector<std::string> entries;

			// critical section
			{
				std::lock_guard<std::mutex> guard(mut);
				while (!q.empty())
				{
					entries.push_back(q.front());
					q.pop();
				}
			}

			// for any entries pulled out of the log queue
			for (auto& s : entries)
			{
				bool sent = false;
				while (!sent)
				{
					try
					{
						// minor delay between each send
						std::this_thread::sleep_for(std::chrono::milliseconds(5));

						// attempt to send the log text
						socket.send(boost::asio::buffer(s + "\n"));
						sent = true;
					}
					catch (...)
					{
						// on any errors, attempt reconnection
						attempt_connection(socket, endpoint);
					}
				}
			}
		}
	}

	// return a timestamp string for a log entry
	std::string get_timestamp() const
	{
		static const std::string fmt = "%Y/%m/%d %H:%M:%S.";
		const auto now = std::chrono::system_clock::now();
		const auto now_time = std::chrono::system_clock::to_time_t(now);
		const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		std::stringstream ss;
		ss << std::put_time(std::localtime(&now_time), fmt.c_str()) << std::setfill('0') << std::setw(3) << now_ms.count() << " - ";
		return ss.str();
	}

public:

	// get singleton instance
	static logger& instance()
	{
		static logger lgr;
		return lgr;
	}

	// push a log entry into the synchornized queue
	void log(const std::string& logtext)
	{
		const std::string ts_logtext = get_timestamp() + logtext;

		std::lock_guard<std::mutex> guard(m_mutex);
		m_queue.push(ts_logtext);
	}
};