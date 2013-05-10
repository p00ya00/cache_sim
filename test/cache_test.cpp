#include <iostream>
#include <iomanip>
#include <cassert>
using namespace std;

#define BOOST_TEST_MODULE CacheTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#define private public //for testing!!
#include <associativity.hpp>
#include <replacement_policy.hpp>
#include "cache.hpp"
#include "miss_handler.hpp"
#include "main_mem_mock.hpp"

struct FullLruWritebackCacheFixture
{
	FullLruWritebackCacheFixture()
	: config(10 * CACHE_LINE_SIZE), cache(config, mm)
	{}

	void fillCache()
	{
		cache.load(adr1_0x005381aa);
		cache.load(adr2_0xff001238);
		cache.load(adr3_0x369146aa);
		cache.load(adr4_0x987654ff);
		cache.load(adr5_0xcccccccc);
		cache.load(adr6_0x99990034);
		cache.load(adr7_0x00000000);
		cache.load(adr8_0xfdeac982);
		cache.load(adr9_0x99887766);
		cache.load(adr10_0x11111111);
	}

	typedef CacheConfiguration<
			LeastRecentlyUsed, FullAssociative, WRITE_BACK, MainMemMock
			> CType;
	MainMemMock mm;
	CType config;
	Cache<CType> cache;
	static const Address
			adr1_0x005381aa = 0x005381aa, adr2_0xff001238 = 0xff001238,
			adr3_0x369146aa = 0x369146aa, adr4_0x987654ff = 0x987654ff,
			adr5_0xcccccccc = 0xcccccccc, adr6_0x99990034 = 0x99990034,
			adr7_0x00000000 = 0x00000000, adr8_0xfdeac982 = 0xfdeac982,
			adr9_0x99887766 = 0x99887766, adr10_0x11111111 = 0x11111111,
			adr11_0x005381ff = 0x005381ff, adr12_0xbbbbbbbb = 0xbbbbbbbb,
			adr13_0x13660507 = 0x13660507, adr14_0x19870729 = 0x19870729;
};

struct FullLruWritethroughCacheFixture
{
	FullLruWritethroughCacheFixture()
	: config(10 * CACHE_LINE_SIZE), cache(config, mm)
	{}

	void fillCache()
	{
		cache.load(adr1_0x005381aa);
		cache.load(adr2_0xff001238);
		cache.load(adr3_0x369146aa);
		cache.load(adr4_0x987654ff);
		cache.load(adr5_0xcccccccc);
		cache.load(adr6_0x99990034);
		cache.load(adr7_0x00000000);
		cache.load(adr8_0xfdeac982);
		cache.load(adr9_0x99887766);
		cache.load(adr10_0x11111111);
	}

	typedef CacheConfiguration<
			LeastRecentlyUsed, FullAssociative, WRITE_THROUGH, MainMemMock
			> CType;
	MainMemMock mm;
	CType config;
	Cache<CType> cache;
	static const Address
			adr1_0x005381aa = 0x005381aa, adr2_0xff001238 = 0xff001238,
			adr3_0x369146aa = 0x369146aa, adr4_0x987654ff = 0x987654ff,
			adr5_0xcccccccc = 0xcccccccc, adr6_0x99990034 = 0x99990034,
			adr7_0x00000000 = 0x00000000, adr8_0xfdeac982 = 0xfdeac982,
			adr9_0x99887766 = 0x99887766, adr10_0x11111111 = 0x11111111,
			adr11_0x005381ff = 0x005381ff, adr12_0xbbbbbbbb = 0xbbbbbbbb,
			adr13_0x13660507 = 0x13660507, adr14_0x19870729 = 0x19870729;
};

BOOST_AUTO_TEST_SUITE(writeBackCache)

BOOST_FIXTURE_TEST_CASE(singleLoadMiss, FullLruWritebackCacheFixture)
{
	BOOST_CHECK(cache.load(adr1_0x005381aa) == MISS);
	BOOST_CHECK_EQUAL(cache.accessCount, 1);
	BOOST_CHECK_EQUAL(cache.hitCount, 0);
}

BOOST_FIXTURE_TEST_CASE(singleLoadHit, FullLruWritebackCacheFixture)
{
	cache.load(adr1_0x005381aa);
	BOOST_CHECK(cache.load(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.accessCount, 2);
	BOOST_CHECK_EQUAL(cache.hitCount, 1);
	BOOST_CHECK_EQUAL(mm.loadCount, 1);
}

BOOST_FIXTURE_TEST_CASE(multipleLoad, FullLruWritebackCacheFixture)
{
	BOOST_CHECK(cache.load(adr1_0x005381aa) == MISS);
	BOOST_CHECK(cache.load(adr2_0xff001238) == MISS);
	BOOST_CHECK(cache.load(adr3_0x369146aa) == MISS);
	BOOST_CHECK(cache.load(adr4_0x987654ff) == MISS);

	BOOST_CHECK(cache.load(adr3_0x369146aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.table.size(), 4);
	BOOST_CHECK_EQUAL(cache.accessCount, 5);
	BOOST_CHECK_EQUAL(cache.hitCount, 1);
	BOOST_CHECK_EQUAL(mm.loadCount, 4);
}

BOOST_FIXTURE_TEST_CASE(fullCacheEvacuation, FullLruWritebackCacheFixture)
{
	fillCache();
	BOOST_CHECK(cache.load(adr12_0xbbbbbbbb) == MISS);
	BOOST_CHECK_EQUAL(mm.loadCount, 11);
	BOOST_CHECK_EQUAL(mm.storeCount, 0);
}

BOOST_FIXTURE_TEST_CASE(fullCacheEvacuateDirty, FullLruWritebackCacheFixture)
{
	fillCache();
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	fillCache();

	BOOST_CHECK(cache.load(adr11_0x005381ff) == MISS);
	BOOST_CHECK_EQUAL(cache.table.size(), 10);
	BOOST_CHECK_EQUAL(mm.loadCount, 11);
	BOOST_CHECK_EQUAL(mm.storeCount, 1);
}

BOOST_FIXTURE_TEST_CASE(singleStoreMiss, FullLruWritebackCacheFixture)
{
	BOOST_CHECK(cache.store(adr1_0x005381aa) == MISS);
	BOOST_CHECK_EQUAL(cache.accessCount, 1);
	BOOST_CHECK_EQUAL(cache.hitCount, 0);
	BOOST_CHECK_EQUAL(mm.loadCount, 1);
	BOOST_CHECK_EQUAL(mm.storeCount, 0);
}

BOOST_FIXTURE_TEST_CASE(singleStoreHit, FullLruWritebackCacheFixture)
{
	BOOST_CHECK(cache.store(adr1_0x005381aa) == MISS);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.accessCount, 2);
	BOOST_CHECK_EQUAL(cache.hitCount, 1);
	BOOST_CHECK_EQUAL(mm.loadCount, 1);
	BOOST_CHECK_EQUAL(mm.storeCount, 0);
}

BOOST_FIXTURE_TEST_CASE(multipleStore, FullLruWritebackCacheFixture)
{
	BOOST_CHECK(cache.store(adr1_0x005381aa) == MISS);
	BOOST_CHECK(cache.store(adr2_0xff001238) == MISS);
	BOOST_CHECK(cache.store(adr3_0x369146aa) == MISS);

	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.accessCount, 6);
	BOOST_CHECK_EQUAL(cache.hitCount, 3);
	BOOST_CHECK_EQUAL(mm.loadCount,3);
	BOOST_CHECK_EQUAL(mm.storeCount, 0);
}

BOOST_FIXTURE_TEST_CASE(multipleStoreEvacuateDirty, FullLruWritebackCacheFixture)
{

	fillCache();
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	fillCache();

	BOOST_CHECK(cache.load(adr13_0x13660507) == MISS);
	BOOST_CHECK_EQUAL(cache.table.size(), 10);
	BOOST_CHECK_EQUAL(mm.loadCount, 11);
	BOOST_CHECK_EQUAL(mm.storeCount, 1);
}

BOOST_AUTO_TEST_SUITE_END()

//////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(writeThroughCache)

BOOST_FIXTURE_TEST_CASE(singleLoadMiss, FullLruWritethroughCacheFixture)
{
	BOOST_CHECK(cache.load(adr1_0x005381aa) == MISS);
	BOOST_CHECK_EQUAL(cache.accessCount, 1);
	BOOST_CHECK_EQUAL(cache.hitCount, 0);
}

BOOST_FIXTURE_TEST_CASE(singleLoadHit, FullLruWritethroughCacheFixture)
{
	cache.load(adr1_0x005381aa);
	BOOST_CHECK(cache.load(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.accessCount, 2);
	BOOST_CHECK_EQUAL(cache.hitCount, 1);
	BOOST_CHECK_EQUAL(mm.loadCount, 1);
}

BOOST_FIXTURE_TEST_CASE(multipleLoad, FullLruWritethroughCacheFixture)
{
	BOOST_CHECK(cache.load(adr1_0x005381aa) == MISS);
	BOOST_CHECK(cache.load(adr2_0xff001238) == MISS);
	BOOST_CHECK(cache.load(adr3_0x369146aa) == MISS);
	BOOST_CHECK(cache.load(adr4_0x987654ff) == MISS);

	BOOST_CHECK(cache.load(adr3_0x369146aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.table.size(), 4);
	BOOST_CHECK_EQUAL(cache.accessCount, 5);
	BOOST_CHECK_EQUAL(cache.hitCount, 1);
	BOOST_CHECK_EQUAL(mm.loadCount, 4);
}

BOOST_FIXTURE_TEST_CASE(fullCacheEvacuation, FullLruWritethroughCacheFixture)
{
	fillCache();
	BOOST_CHECK(cache.load(adr12_0xbbbbbbbb) == MISS);
	BOOST_CHECK_EQUAL(mm.loadCount, 11);
	BOOST_CHECK_EQUAL(mm.storeCount, 0);
}

BOOST_FIXTURE_TEST_CASE(fullCacheEvacuateDirty, FullLruWritethroughCacheFixture)
{
	fillCache();
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	fillCache();

	BOOST_CHECK(cache.load(adr11_0x005381ff) == MISS);
	BOOST_CHECK_EQUAL(cache.table.size(), 10);
	BOOST_CHECK_EQUAL(mm.loadCount, 11);
	BOOST_CHECK_EQUAL(mm.storeCount, 1);
}

BOOST_FIXTURE_TEST_CASE(singleStoreMiss, FullLruWritethroughCacheFixture)
{
	BOOST_CHECK(cache.store(adr1_0x005381aa) == MISS);
	BOOST_CHECK_EQUAL(cache.accessCount, 1);
	BOOST_CHECK_EQUAL(cache.hitCount, 0);
	BOOST_CHECK_EQUAL(mm.loadCount, 1);
	BOOST_CHECK_EQUAL(mm.storeCount, 0);
}

BOOST_FIXTURE_TEST_CASE(singleStoreHit, FullLruWritethroughCacheFixture)
{
	BOOST_CHECK(cache.store(adr1_0x005381aa) == MISS);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.accessCount, 2);
	BOOST_CHECK_EQUAL(cache.hitCount, 1);
	BOOST_CHECK_EQUAL(mm.loadCount, 1);
	BOOST_CHECK_EQUAL(mm.storeCount, 1);
}

BOOST_FIXTURE_TEST_CASE(multipleStore, FullLruWritethroughCacheFixture)
{
	BOOST_CHECK(cache.store(adr1_0x005381aa) == MISS);
	BOOST_CHECK(cache.store(adr2_0xff001238) == MISS);
	BOOST_CHECK(cache.store(adr3_0x369146aa) == MISS);

	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK_EQUAL(cache.accessCount, 6);
	BOOST_CHECK_EQUAL(cache.hitCount, 3);
	BOOST_CHECK_EQUAL(mm.loadCount,3);
	BOOST_CHECK_EQUAL(mm.storeCount, 3);
}

BOOST_FIXTURE_TEST_CASE(multipleStoreEvacuateDirty, FullLruWritethroughCacheFixture)
{

	fillCache();
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	BOOST_CHECK(cache.store(adr1_0x005381aa) == L1HIT);
	fillCache();

	BOOST_CHECK(cache.load(adr13_0x13660507) == MISS);
	BOOST_CHECK_EQUAL(cache.table.size(), 10);
	BOOST_CHECK_EQUAL(mm.loadCount, 11);
	BOOST_CHECK_EQUAL(mm.storeCount, 2);
}

BOOST_AUTO_TEST_SUITE_END()
