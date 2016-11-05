/*
 * MinMaxCheck_Test.cpp
 *
 *  Created on: Jun 24, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../StreamProcessors/MinMaxCheck.hpp"


BOOST_AUTO_TEST_SUITE(MinMaxCheck_Test)


BOOST_AUTO_TEST_CASE(testConstruction)
{
	int samplesPerSegment = 2;
	int numSegments = 2;
	MinMaxCheck dut(samplesPerSegment, numSegments);
}


BOOST_AUTO_TEST_CASE(test1SamplePerSegment_Increasing)
{
	int samplesPerSegment = 1;
	int numSegments = 2;
	MinMaxCheck dut(samplesPerSegment, numSegments);

	dut.check(1);
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(1, dut.getMax());

	dut.check(2);
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(2, dut.getMax());

	dut.check(3);
	BOOST_CHECK_EQUAL(2, dut.getMin());
	BOOST_CHECK_EQUAL(3, dut.getMax());

	dut.check(4);
	BOOST_CHECK_EQUAL(3, dut.getMin());
	BOOST_CHECK_EQUAL(4, dut.getMax());
}


BOOST_AUTO_TEST_CASE(test1SamplePerSegment_Decreasing)
{
	int samplesPerSegment = 1;
	int numSegments = 2;
	MinMaxCheck dut(samplesPerSegment, numSegments);

	dut.check(4);
	BOOST_CHECK_EQUAL(4, dut.getMin());
	BOOST_CHECK_EQUAL(4, dut.getMax());

	dut.check(3);
	BOOST_CHECK_EQUAL(3, dut.getMin());
	BOOST_CHECK_EQUAL(4, dut.getMax());

	dut.check(2);
	BOOST_CHECK_EQUAL(2, dut.getMin());
	BOOST_CHECK_EQUAL(3, dut.getMax());

	dut.check(1);
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(2, dut.getMax());
}


BOOST_AUTO_TEST_CASE(test2SamplesPerSegment_Increasing)
{
	int samplesPerSegment = 2;
	int numSegments = 2;
	MinMaxCheck dut(samplesPerSegment, numSegments);

	dut.check(1);
	// | segment 0 | segment 1 |
	// |   1       |           |
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(1, dut.getMax());

	dut.check(2);
	// | segment 0 | segment 1 |
	// |   1   2   |           |
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(2, dut.getMax());

	dut.check(3);
	// | segment 0 | segment 1 |
	// |   1   2   |   3       |
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(3, dut.getMax());

	dut.check(4);
	// | segment 0 | segment 1 |
	// |   1   2   |   3   4   |
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(4, dut.getMax());

	dut.check(5);
	// | segment 0 | segment 1 | Unfinished |
	// |   1   2   |   3   4   |   5        |
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(5, dut.getMax());

	dut.check(6);
	// | segment 0 | segment 1 |
	// |   3   4   |   5   6   |
	BOOST_CHECK_EQUAL(3, dut.getMin());
	BOOST_CHECK_EQUAL(6, dut.getMax());

	dut.check(7);
	// | segment 0 | segment 1 | Unfinished |
	// |   3   4   |   5   6   |   7
	BOOST_CHECK_EQUAL(3, dut.getMin());
	BOOST_CHECK_EQUAL(7, dut.getMax());
}


BOOST_AUTO_TEST_CASE(test2SamplesPerSegment_Decreasing)
{
	int samplesPerSegment = 2;
	int numSegments = 2;
	MinMaxCheck dut(samplesPerSegment, numSegments);

	dut.check(7);
	// | segment 0 | segment 1 |
	// |   7       |           |
	BOOST_CHECK_EQUAL(7, dut.getMin());
	BOOST_CHECK_EQUAL(7, dut.getMax());

	dut.check(6);
	// | segment 0 | segment 1 |
	// |   7   6   |           |
	BOOST_CHECK_EQUAL(6, dut.getMin());
	BOOST_CHECK_EQUAL(7, dut.getMax());

	dut.check(5);
	// | segment 0 | segment 1 |
	// |   7   6   |   5       |
	BOOST_CHECK_EQUAL(5, dut.getMin());
	BOOST_CHECK_EQUAL(7, dut.getMax());

	dut.check(4);
	// | segment 0 | segment 1 |
	// |   7   6   |   5   4   |
	BOOST_CHECK_EQUAL(4, dut.getMin());
	BOOST_CHECK_EQUAL(7, dut.getMax());

	dut.check(3);
	// | segment 0 | segment 1 | Unfinished |
	// |   7   6   |   5   4   |   3
	BOOST_CHECK_EQUAL(3, dut.getMin());
	BOOST_CHECK_EQUAL(7, dut.getMax());

	dut.check(2);
	// | segment 0 | segment 1 |
	// |   5   4   |   3   2   |
	BOOST_CHECK_EQUAL(2, dut.getMin());
	BOOST_CHECK_EQUAL(5, dut.getMax());

	dut.check(1);
	// | segment 0 | segment 1 | Unfinished |
	// |   5   4   |   3   2   |   1
	BOOST_CHECK_EQUAL(1, dut.getMin());
	BOOST_CHECK_EQUAL(5, dut.getMax());
}

BOOST_AUTO_TEST_SUITE_END()
