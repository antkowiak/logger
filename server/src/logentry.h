#pragma once

//
// logentry.h - Log entry in the queue
//
// Written by Ryan Antkowiak (antkowiak@gmail.com)
//

#include <string>
#include <utility>

// log entry in the queue
struct logentry
{
private:
	std::string logtext;

public:
	logentry() = default;
	~logentry() = default;
	logentry(const logentry&) = default;
	logentry(logentry&&) noexcept = default;
	logentry& operator = (const logentry&) = default;
	logentry& operator = (logentry&&) noexcept = default;

	// constructor taking a const ref to a string
	logentry(const std::string& data)
		: logtext(data)
	{
	}

	// constructor taking r-value ref to a string
	logentry(std::string&& data)
		: logtext(std::move(data))
	{
	}

	// return string representation of this log entry
	std::string to_string() const
	{
		return logtext;
	}
};
