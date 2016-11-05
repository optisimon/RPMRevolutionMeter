/*
 * SlidingAverager_Test.cpp
 *
 *  Created on: Jun 24, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>


#include "../StreamProcessors/SlidingAverager.hpp"


BOOST_AUTO_TEST_SUITE(SlidingAverager_Test)


BOOST_AUTO_TEST_CASE(testConstruction)
{
	int windowSize = 5;
	SlidingAverager dut(windowSize);
}


BOOST_AUTO_TEST_CASE(testWindowSize1)
{
	int windowSize = 1;
	SlidingAverager dut(windowSize);

	dut.push(1.0);
	BOOST_CHECK_EQUAL(1.0, dut.getAverage());

	dut.push(2.0);
	BOOST_CHECK_EQUAL(2.0, dut.getAverage());

	dut.push(3.0);
	BOOST_CHECK_EQUAL(3.0, dut.getAverage());
}


BOOST_AUTO_TEST_CASE(testWindowSize2)
{
	int windowSize = 2;
	SlidingAverager dut(windowSize);

	dut.push(1.0);
	BOOST_CHECK_EQUAL(1.0, dut.getAverage());

	dut.push(3.0);
	BOOST_CHECK_EQUAL(2.0, dut.getAverage());

	dut.push(5.0);
	BOOST_CHECK_EQUAL(4.0, dut.getAverage());
}

BOOST_AUTO_TEST_SUITE_END()
