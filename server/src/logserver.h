#pragma once

//
// logserver.h - Log server that accepts connections and creates sessions.
//
// Written by Ryan Antkowiak 
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
#include "logsession.h"

// the log server that accepts connections and creates sessions
class logserver
{
public:
    // constructor
    logserver(boost::asio::io_service& io_service, short port, logqueue& q)
        : m_service(io_service),
        m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
        m_queue(q)
    {
        // create a listen session
        logsession* new_session = new logsession(m_service, m_queue);
        m_sessions.push_back(new_session);

        // asynchornously accept clients
        m_acceptor.async_accept(new_session->socket(),
            boost::bind(&logserver::handle_accept, this, new_session,
                boost::asio::placeholders::error));
    }

    // accept a client connection
    void handle_accept(logsession* new_session, const boost::system::error_code& error)
    {
        if (!error)
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            // create the reader thread for this session
            std::thread t(&logsession::start, new_session);
            m_threadpool.emplace_back(std::move(t));
            new_session = new logsession(m_service, m_queue);
            m_sessions.push_back(new_session);

            m_acceptor.async_accept(new_session->socket(),
                boost::bind(&logserver::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
        }
        else
        {
            delete new_session;
        }
    }

    // destructor
    ~logserver()
    {
        // join all threads
        for (auto& t : m_threadpool)
            t.join();

        // delete all sessions
        for (auto& s : m_sessions)
            if (s != nullptr)
                delete s;
    }

private:
    // boost asio service
    boost::asio::io_service& m_service;

    // boost asio acceptor
    boost::asio::ip::tcp::acceptor m_acceptor;

    // reference to the queue of log entries
    logqueue& m_queue;

    // mutex to protect access to the threadpool and the session vector
    std::mutex m_mutex;

    // thread pool of connected clients/sessions
    std::vector<std::thread> m_threadpool;

    // cache of session objects
    std::vector<logsession*> m_sessions;
};
