/*
 * Stopwatch.hpp
 *
 *  Created on: Sep 3, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#pragma once

#include <assert.h>
#include <time.h>

class Stopwatch {
public:
	Stopwatch()
	{
		int rc = clock_gettime(CLOCK_MONOTONIC, &_start);
		assert(rc == 0 && "clock_gettime failed");
	}

	void restart()
	{
		int rc = clock_gettime(CLOCK_MONOTONIC, &_start);
		assert(rc == 0 && "clock_gettime failed");
	}

	void getElapsed(uint64_t* secs, uint64_t* nsecs) const
	{
		assert(secs && nsecs);

		struct timespec now;

		int rc = clock_gettime(CLOCK_MONOTONIC, &now);
		assert(rc == 0 && "clock_gettime failed");

		if (_start.tv_nsec > now.tv_nsec)
		{
			now.tv_nsec += 1000000000L;
			now.tv_sec -= 1;
		}

		*nsecs = now.tv_nsec - _start.tv_nsec;
		*secs = now.tv_sec - _start.tv_sec;
	}
private:
	struct timespec _start;
};
