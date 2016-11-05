/*
 * MinMaxCheck.hpp
 *
 *  Created on: Jun 24, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#pragma once

#include <stdlib.h>

#include <limits>
#include <deque>

/**
 * Determines minimum and maximum value that has been passed during
 * the at most the last samplesPerSegment * (numSegments + 1) - 1  samples.
 * Actual number will gitter with a number of samples of samplesPerSegment
 *
 * Works by having a deque of min/max values for each previously checked segment.
 * In addition to those segments, the current samples (on the way to form a segment)
 * will also contribute  to the reported min / max value.
 *
 * That way we can have a rough sliding window which gives useful
 * min/max values for a longer time span without having to store every single value,
 * or having to resort to leaky bucket filtering.
 *
 * */
class MinMaxCheck {
public:
	MinMaxCheck(size_t samplesPerSegment, size_t numSegments) :
		_minValueInSegments(std::numeric_limits<int16_t>::max()),
		_maxValueInSegments(std::numeric_limits<int16_t>::min()),
		_currentMin(std::numeric_limits<int16_t>::max()),
		_currentMax(std::numeric_limits<int16_t>::min()),
		_sampleInSegmentCntr(0),
		_samplesPerSegment(samplesPerSegment),
		_numSegments(numSegments)
	{ }

	void check(int16_t sample)
	{
		if (_sampleInSegmentCntr == 0)
		{
			startNewSegment(sample);
			_sampleInSegmentCntr = 0;
		}

		if (sample > _currentMax) { _currentMax = sample; }
		if (sample < _currentMin) { _currentMin = sample; }
		if (sample > _maxValueInSegments) { _maxValueInSegments = sample; }
		if (sample < _minValueInSegments) { _minValueInSegments = sample; }
		_sampleInSegmentCntr++;

		if (_sampleInSegmentCntr >= _samplesPerSegment)
		{
			endNewSegment();
			_sampleInSegmentCntr = 0;
		}
	}

	int16_t getMin() const { return _minValueInSegments; }
	int16_t getMax() const { return _maxValueInSegments; }

//	void printDebugInfo()
//	{
//
//		std::cout
//		<< "_sampleInSegmentCntr=" << _sampleInSegmentCntr << "\n"
//		<< "_currentMin=" << _currentMin << "\n"
//		<< "_currentMax=" << _currentMax << "\n"
//		<< "_minValueInSegments=" << _minValueInSegments << "\n"
//		<< "_maxValueInSegments=" << _maxValueInSegments << "\n";
//		for (size_t i = 0; i < _minMax.size(); i++)
//		{
//			std::cout << "| Segment " << i << " |";
//		}
//		std::cout << "| Unfinished |\n";
//
//		for (const auto& minmax : _minMax)
//		{
//			std::cout << "|   " << minmax.min << "   " << minmax.max << "   |";
//		}
//		std::cout << "|   " << _currentMin << "   " << _currentMax << "   |";
//		std::cout << "\n\n";
//	}

private:
	int16_t _minValueInSegments;
	int16_t _maxValueInSegments;
	int16_t _currentMin;
	int16_t _currentMax;
	size_t _sampleInSegmentCntr;
	const size_t _samplesPerSegment;
	const size_t _numSegments;
	struct MinMax {
		int16_t min;
		int16_t max;
		MinMax(int min, int max) : min(min), max(max)
		{}
	};
	std::deque<MinMax> _minMax;

	void endNewSegment()
	{
		_minMax.emplace_back(_currentMin, _currentMax);
		if (_minMax.size() > _numSegments)
		{
			_minMax.pop_front();
		}

		_minValueInSegments = std::numeric_limits<int16_t>::max();
		_maxValueInSegments = std::numeric_limits<int16_t>::min();
		for (const auto& minmax : _minMax)
		{
			if (minmax.max > _maxValueInSegments)
			{
				_maxValueInSegments = minmax.max;
			}
			if (minmax.min < _minValueInSegments)
			{
				_minValueInSegments = minmax.min;
			}
		}

		_currentMin = std::numeric_limits<int16_t>::max();
		_currentMax = std::numeric_limits<int16_t>::min();

	}
	void startNewSegment(int16_t sample)
	{
		_currentMin = _currentMax = sample;
	}
};
