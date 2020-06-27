
//
// main.cpp - Contains main function to run the logging server.
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
#include <iostream>
#include <string>
#include <thread>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "logentry.h"
#include "logqueue.h"
#include "logserver.h"
#include "logsession.h"

// thread to print all entries in the log queue
void print_thread(logqueue & q)
{
    // loop forever
    while (true)
    {
        // delay, as to not cause contention in locking the queue
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // pop all log entries out of the queue
        auto entries = q.pop();

        // for each log entry, print it
        for (auto& e : entries)
            std::cout << e.to_string() << std::endl;
    }
}

int main(int argc, const char* argv[])
{
    // the log queue
    logqueue q;

    // printer thread, which will display all log entries
    std::thread printer(print_thread, std::ref(q));

    // default listen port
    short port = 8080;

    try
    {
        // check for too many arguments
        if (argc > 2)
        {
            std::cout << "Usage:  logserver <port>" << std::endl;
            return 0;
        }

        // if a different port number was provided, use it
        if (argc == 2)
            port = std::atoi(argv[1]);

        // create the boost asio service
        boost::asio::io_service io_service;

        // create the log server
        logserver s(io_service, port, q);

        // run the log server
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // join the printer thread
    printer.join();

    return 0;
}

#pragma warning( pop )
