#pragma once

//
// logsession.h - The connection of a logging session.
//
// Written by Ryan Antkowiak (antkowiak@gmail.com)
//

#define _CRT_SECURE_NO_DEPRECATE
#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

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

#include <cstdint>
#include <cstdlib>
#include <string>
#include <thread>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "logentry.h"
#include "logqueue.h"

// the connection of a logging session
class logsession
{
public:
    // constructor
    logsession(boost::asio::io_service& svc, logqueue& q)
        : m_socket(svc),
        m_queue(q)
    {
    }

    // return a reference to the socket
    boost::asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }

    // starts the session (reads log entries and prints them)
    void start()
    {
        try
        {
            // loop forever
            while (true)
            {
                boost::asio::streambuf b;
                // read one log entry from the socket (log entries delimited by newlines)
                boost::asio::read_until(m_socket, b, "\n");

                std::istream is(&b);
                std::string line;
                std::getline(is, line);

                // push the log entry into the queue to be printed
                if (!line.empty())
                    m_queue.push(logentry(std::move(line)));
            }
        }
        catch (std::exception&)
        {
        }
    }

private:
    // the socket for this session
    boost::asio::ip::tcp::socket m_socket;

    // reference to the synchornized queue of incoming log messages
    logqueue& m_queue;
};
