/*
 * CappedStorageWaveform_Test.cpp
 *
 *  Created on: Jun 25, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../StreamProcessors/CappedStorageWaveform.hpp"

#define BRACED_INIT_LIST(...) {__VA_ARGS__}
/**
 * Usage:
 * std::vector<int16_t> right = ...;
 * CHECK_VECTORS((0, 1, 2), right);
 */
#define CHECK_VECTORS(L, R) { \
	std::vector<int16_t> left(BRACED_INIT_LIST L); \
	BOOST_REQUIRE_EQUAL(left.size(), R.size()); \
	for (size_t i = 0; i < left.size(); i++) { \
		BOOST_CHECK_EQUAL(left[i], R[i]); \
	} \
}

BOOST_AUTO_TEST_SUITE(CappedStorageWaveform_Test)


BOOST_AUTO_TEST_CASE(construction)
{
	CappedStorageWaveform w;
}

BOOST_AUTO_TEST_CASE(beforeCompaction)
{
	CappedStorageWaveform w(4);

	w.push(0);
	BOOST_CHECK_EQUAL(1, w.getWaveform().size());
	CHECK_VECTORS((0), w.getWaveform());
	w.push(1);
	BOOST_CHECK_EQUAL(2, w.getWaveform().size());
	CHECK_VECTORS((0, 1), w.getWaveform());
	w.push(2);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 1, 2), w.getWaveform());
	w.push(3);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 1, 2, 3), w.getWaveform());
}

BOOST_AUTO_TEST_CASE(after1Compaction)
{
	CappedStorageWaveform w(4);

	w.push(0);
	BOOST_CHECK_EQUAL(1, w.getWaveform().size());
	CHECK_VECTORS((0), w.getWaveform());
	w.push(1);
	BOOST_CHECK_EQUAL(2, w.getWaveform().size());
	CHECK_VECTORS((0, 1), w.getWaveform());
	w.push(2);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 1, 2), w.getWaveform());
	w.push(3);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 1, 2, 3), w.getWaveform());

	w.push(4); // Triggers first compaction
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 2, 4), w.getWaveform());

	w.push(5);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 2, 4), w.getWaveform());

	w.push(6);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 2, 4, 6), w.getWaveform());
}

BOOST_AUTO_TEST_CASE(severalCompactions)
{
	CappedStorageWaveform w(4);

	w.push(0);
	BOOST_CHECK_EQUAL(1, w.getWaveform().size());
	CHECK_VECTORS((0), w.getWaveform());
	w.push(1);
	BOOST_CHECK_EQUAL(2, w.getWaveform().size());
	CHECK_VECTORS((0, 1), w.getWaveform());
	w.push(2);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 1, 2), w.getWaveform());
	w.push(3);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 1, 2, 3), w.getWaveform());


	w.push(4); // Triggers first compaction
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 2, 4), w.getWaveform());

	w.push(5);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 2, 4), w.getWaveform());

	w.push(6);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 2, 4, 6), w.getWaveform());


	w.push(7); // Triggers second compaction
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 2, 4, 6), w.getWaveform());

	w.push(8);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8), w.getWaveform());

	w.push(9);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8), w.getWaveform());

	w.push(10);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8), w.getWaveform());

	w.push(11);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8), w.getWaveform());

	w.push(12);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8, 12), w.getWaveform());


	w.push(13);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8, 12), w.getWaveform());

	w.push(14);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8, 12), w.getWaveform());

	w.push(15);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 4, 8, 12), w.getWaveform());


	w.push(16); // Triggers third compaction
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());

	w.push(17);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());

	w.push(18);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());

	w.push(19);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());

	w.push(20);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());

	w.push(21);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());

	w.push(22);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());

	w.push(23);
	BOOST_CHECK_EQUAL(3, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16), w.getWaveform());


	w.push(24);
	BOOST_CHECK_EQUAL(4, w.getWaveform().size());
	CHECK_VECTORS((0, 8, 16, 24), w.getWaveform());
}

BOOST_AUTO_TEST_SUITE_END();
