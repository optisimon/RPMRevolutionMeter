/*
 * SlidingAverager.hpp
 *
 *  Created on: Jun 25, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#pragma once

#include <stdlib.h>

#include <deque>

class SlidingAverager {
public:
	SlidingAverager(size_t windowSize) : _windowSize(windowSize)
	{ }

	void push(double value)
	{
		_values.push_back(value);
		if (_values.size() > _windowSize)
		{
			_values.pop_front();
		}
	}

	/**
	 * Get sliding average of up to the last windowSize samples.
	 * @warning returns 0 when no samples ever presented to this class.
	 */
	double getAverage() const
	{
		if (_values.size() == 0)
		{
			return 0;
		}

		double sum = 0;

		for (const auto& value : _values)
		{
			sum += value;
		}
		return sum / _values.size();
	}

private:
	const size_t _windowSize;
	std::deque<double> _values;
};
