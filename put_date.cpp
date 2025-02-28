#ifndef PUTDATE
#define PUTDATE
#include <iostream>
#include <ctime>
#include <cstdio>

namespace std
{
	// A simple manipulator type that holds a reference to a std::tm and a format string.
	struct put_time_t
	{
		const std::tm &tm;
		const char *fmt;

		put_time_t(const std::tm &t, const char *f) : tm(t), fmt(f) {}
	};

	// Overload the stream insertion operator for put_time_t.
	std::ostream &operator<<(std::ostream &os, const put_time_t &pt)
	{
		char buffer[256];
		if (std::strftime(buffer, sizeof(buffer), pt.fmt, &pt.tm))
		{
			os << buffer;
		}
		return os;
	}

	// Helper function to create a put_time_t object.
	put_time_t put_time(const std::tm &tm, const char *fmt)
	{
		return put_time_t(tm, fmt); // Use constructor syntax instead of brace initialization.
	}
}

#endif