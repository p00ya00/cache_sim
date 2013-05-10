#define BOOST_TEST_MODULE PrefetcherTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

//#include <iostream>
//#include <iomanip>
//using namespace std;
//#include <cassert>

#define private public
#include <stream_prefetcher.hpp>
#include <stride_prefetcher.hpp>

struct CacheHistoryFixture
{
	//sequence threshold is 8
	CacheHistoryFixture()
	: cachelineSize(64), streamer(8, &history, cachelineSize)
	, toFetch(nullptr), seqStart(nullptr)
	{}

	short cachelineSize;
	StreamPrefetcher streamer;
	AccessList history;
	void *toFetch, *seqStart;
};

struct StridePrefetcherFixture
{
	StridePrefetcherFixture()
	: cachelineSize(64), stride(cachelineSize, 2048, 3, 4), toFetch(0x0)
	{}

	short cachelineSize;
	StridePrefetcher stride;
	Address toFetch;
};

BOOST_AUTO_TEST_SUITE(AscendingPrefetcher)

BOOST_FIXTURE_TEST_CASE(emptyHistory, CacheHistoryFixture)
{
	BOOST_CHECK(history.size == 0);
	BOOST_REQUIRE(streamer.ascendingPrefetch(&toFetch, &seqStart) == false);
	BOOST_CHECK(toFetch == nullptr);
}

BOOST_FIXTURE_TEST_CASE(notEnoughHistory, CacheHistoryFixture)
{
	history.add(0x0000);
	history.add(0x0048);
	history.add(0x008f);
	history.add(0x00c3);
	BOOST_CHECK(history.size == 4);
	BOOST_REQUIRE(streamer.ascendingPrefetch(&toFetch, &seqStart) == false);
	BOOST_CHECK(toFetch == NULL);
}

BOOST_FIXTURE_TEST_CASE(prefetch1, CacheHistoryFixture)
{
	history.add(0x0000);
	history.add(0x0048);
	history.add(0x008f);
	history.add(0x00c3);
	history.add(0x0100);
	history.add(0x0148);
	history.add(0x018f);
	history.add(0x01c3);
	BOOST_REQUIRE(streamer.ascendingPrefetch(&toFetch, &seqStart) == true);
	BOOST_CHECK(toFetch != nullptr);
	BOOST_CHECK((Address)toFetch == 0x01c3 + cachelineSize);
}

BOOST_FIXTURE_TEST_CASE(noPrefetch, CacheHistoryFixture)
{
	history.add(0x0000);
	history.add(0x0048);
	history.add(0x008f);
	history.add(0x00c3);
	history.add(0x0000);
	history.add(0x0148);
	history.add(0x018f);
	history.add(0x01c3);
	BOOST_REQUIRE(streamer.ascendingPrefetch(&toFetch, &seqStart) == false);
	BOOST_CHECK(toFetch == nullptr);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(DescendingPrefetcher)

BOOST_FIXTURE_TEST_CASE(emptyHistory, CacheHistoryFixture)
{
	BOOST_REQUIRE(streamer.descendingPrefetch(&toFetch, &seqStart) == false);
	BOOST_CHECK(toFetch == nullptr);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SpatialPrefetcherTestSiute)

BOOST_AUTO_TEST_CASE(spatial)
{
	//BOOST_REQUIRE(spatialPrefetcher(0x177f) == 0x173f);
	//BOOST_REQUIRE(spatialPrefetcher(0x43b0) == 0x43f0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(StridePrefetcherTestSuite)

BOOST_FIXTURE_TEST_CASE(successfulForwardPrefetch1, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x470, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x4f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x570, &toFetch) == true);
	BOOST_REQUIRE(toFetch == 0x5f0);
}

BOOST_FIXTURE_TEST_CASE(successfulForwardPrefetch2, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x475, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x4ff, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x57a, &toFetch) == true);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x5fa, &toFetch) == true);
	BOOST_REQUIRE(toFetch == 0x67a);
}

BOOST_FIXTURE_TEST_CASE(sameCachelineAccessForward, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff50, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff59, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff5f, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff73, &toFetch) == false);
	//now the stride of size 0x100 begins
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x10073, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x10173, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x10273, &toFetch) == true);
	BOOST_REQUIRE(toFetch = 0x10373);
}

BOOST_FIXTURE_TEST_CASE(sameCachelineAccessBackward, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff56, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff51, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff5f, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xff7e, &toFetch) == false);
	//now the stride of size 0x100 begins
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xfe7e, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xfd75, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xfc41, &toFetch) == true);
	BOOST_REQUIRE(toFetch = 0xfb41);
}

BOOST_FIXTURE_TEST_CASE(oversizedForwardStride, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3333, &toFetch) == false);
	//two strides of size 0x200 occure
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3533, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3733, &toFetch) == false);
	//third stride is bigger than max stride size (2k) and will not be counted as
	//a stride. stride size 0x880 > 2048
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3fb3, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x4833, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x50b3, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x5933, &toFetch) == false);
}

BOOST_FIXTURE_TEST_CASE(oversizedBackwardStride, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3333, &toFetch) == false);
	//two strides of size 0x200 occure
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3133, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x2f33, &toFetch) == false);
	//third stride is bigger than max stride size (2k) and will not be counted as
	//a stride. stride size 0x880 > 2048
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x26b3, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x1e33, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x15b3, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xd33, &toFetch) == false);
}

BOOST_FIXTURE_TEST_CASE(alreadyPrefetchedForwardStride, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x470, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x4f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x570, &toFetch) == true);
	BOOST_REQUIRE(toFetch == 0x5f0);
	//different stride to clear the previous state
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0xeeee, &toFetch) == false);
	//if same prefetch request gets generated, prefetch will not occure
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x3f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x470, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x4f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x570, &toFetch) == false);
}

BOOST_FIXTURE_TEST_CASE(alreadyPrefetchedBackwardStride, StridePrefetcherFixture)
{
	//...
}

BOOST_FIXTURE_TEST_CASE(successfulBackwardPrefetch1, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x5f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x570, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x4f0, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x470, &toFetch) == true);
	BOOST_REQUIRE(toFetch == 0x3f0);
}

BOOST_FIXTURE_TEST_CASE(successfulBackwardPrefetch2, StridePrefetcherFixture)
{
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x67a, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x5fa, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x57a, &toFetch) == false);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x4ff, &toFetch) == true);
	BOOST_REQUIRE(stride.prefetch((void *)0x123, 0x475, &toFetch) == true);
	BOOST_REQUIRE_EQUAL(toFetch, 0x3f5);
}

BOOST_AUTO_TEST_SUITE_END()

//int main()
//{
//	StridePrefetcher stride(64, 2048, 3);
//	Address toFetch;
//
//	assert(stride.prefetch((void *)0x123, 0x3f0, &toFetch) == false);
//	assert(stride.prefetch((void *)0x123, 0x470, &toFetch) == false);
//	assert(stride.prefetch((void *)0x123, 0x4f0, &toFetch) == false);
//	assert(stride.prefetch((void *)0x123, 0x570, &toFetch) == true);
//	assert(toFetch == 0x5f0);
//	//different stride to clear the previous state
//	assert(stride.prefetch((void *)0x123, 0xeeee, &toFetch) == false);
//	//if same prefetch request gets generated, prefetch will not occure
//	assert(stride.prefetch((void *)0x123, 0x3f0, &toFetch) == false);
//	assert(stride.prefetch((void *)0x123, 0x470, &toFetch) == false);
//	assert(stride.prefetch((void *)0x123, 0x4f0, &toFetch) == false);
//	assert(stride.prefetch((void *)0x123, 0x570, &toFetch) == false);
//
//	return 0;
//}
