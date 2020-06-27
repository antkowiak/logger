
//
// main.cpp - Sample usage of logger.h logger client.
//
// Written by Ryan Antkowiak (antkowiak@gmail.com)
//

#define _CRT_SECURE_NO_DEPRECATE
#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

#include <iostream>
#include <string>

#include "logger.h"

int main()
{
    // send 100 initial log entries, in rapid-succession
    for (int i = 0 ; i < 100 ; ++i)
        logger::instance().log("warm up " + std::to_string(i));

    // loop forever
    while(true)
    {
        // read text to send to the log server
        std::cout << "Enter: ";
        std::string line;
        std::getline(std::cin, line);

        // push the log entry into the queue
        logger::instance().log(line);
    }

    return 0;
}
