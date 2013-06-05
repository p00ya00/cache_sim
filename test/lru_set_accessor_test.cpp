#include <iostream>
#include <iomanip>
#include <cassert>
using namespace std;

#define private public // for testing!!
#include <set_accessor.hpp>

#define BOOST_TEST_MODULE lurSetAccessorTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

/**
 * a cache with cache line size of 64 byte and 4 sets with 4-way associativity
 */

CacheConfig conf(4 * 4 * 64, LRU, SET_ASSOC, WRITE_BACK, 1, DATA_CACHE, 64, 4);

Address adr1a_0x06237817 = 0x06237817, adr1b_0x06737827 = 0x06737827,
		adr1c_0x06a37837 = 0x06a37837, adr1d_0x06f37837 = 0x06f37837,
		adr1e_0x87600000 = 0x87600000, adr1f_0xfabed001 = 0xfabed001,

		adr2a_0x98325345 = 0x98325345, adr2b_0x98725355 = 0x98725355,
		adr2c_0xa8725365 = 0xa8725365, adr2d_0xb8725345 = 0xb8725345,
		adr2e_0x5642325f = 0x5642325f,

		adr3a_0xa8115385 = 0xa8115385, adr3b_0xb8005385 = 0xb8005385,
		adr3c_0xa8ff5395 = 0xa8ff5395, adr3d_0xb83453a5 = 0xb83453a5,
		adr3e_0x983724b0 = 0x983724b0,

		adr4a_0x938753c5 = 0x938753c5, adr4b_0xffff53d5 = 0xffff53d5,
		adr4c_0x000053e5 = 0x000053e5, adr4d_0x0eca53f5 = 0x0eca53f5,
		adr4e_0x001fffcc = 0x001fffcc;

struct EmptyCacheFixture
{
	EmptyCacheFixture()
	: lsa(conf)
	{
		table = new CacheLineSet[conf.numberOfSets];
		lsa.setCacheTable(table);
	}

	void printCacheTable()
	{
		for(size_t i = 0; i < lsa.cacheConfig.numberOfSets; ++i)
		{
			cout << "(" << i << ")  size: " << lsa.cacheTable[i].size << "  " << hex;
			for(CacheLineList *cll = lsa.cacheTable[i].set; cll != nullptr; cll = cll->next)
			{
				cout << "[0x" << cll->cacheline->tag << "]";
				if(cll->next != nullptr)
					cout << ",";
			}
			cout << endl;
		}
	}

	CacheTable table;
	SetAccessor lsa;
};

struct FullCacheFixture : EmptyCacheFixture
{
	FullCacheFixture()
	{
		lsa.write(adr1a_0x06237817); lsa.write(adr1b_0x06737827);
		lsa.write(adr1c_0x06a37837); lsa.write(adr1d_0x06f37837);

		lsa.write(adr2a_0x98325345); lsa.write(adr2b_0x98725355);
		lsa.write(adr2c_0xa8725365); lsa.write(adr2d_0xb8725345);

		lsa.write(adr3a_0xa8115385); lsa.write(adr3b_0xb8005385);
		lsa.write(adr3c_0xa8ff5395); lsa.write(adr3d_0xb83453a5);

		lsa.write(adr4a_0x938753c5); lsa.write(adr4b_0xffff53d5);
		lsa.write(adr4c_0x000053e5); lsa.write(adr4d_0x0eca53f5);
	}
};

BOOST_FIXTURE_TEST_CASE(validateEmptyCache, EmptyCacheFixture)
{
	BOOST_REQUIRE(lsa.cacheTable[0].size == 0);
	BOOST_REQUIRE(lsa.cacheTable[0].set == nullptr);
	BOOST_REQUIRE(lsa.cacheTable[1].size == 0);
	BOOST_REQUIRE(lsa.cacheTable[1].set == nullptr);
	BOOST_REQUIRE(lsa.cacheTable[2].size == 0);
	BOOST_REQUIRE(lsa.cacheTable[2].set == nullptr);
	BOOST_REQUIRE(lsa.cacheTable[3].size == 0);
	BOOST_REQUIRE(lsa.cacheTable[3].set == nullptr);

	BOOST_REQUIRE_EQUAL(lsa.locTag(adr1a_0x06237817), 0);
	BOOST_REQUIRE_EQUAL(lsa.locTag(adr2c_0xa8725365), 1);
	BOOST_REQUIRE_EQUAL(lsa.locTag(adr3e_0x983724b0), 2);
	BOOST_REQUIRE_EQUAL(lsa.locTag(adr4b_0xffff53d5), 3);

	BOOST_REQUIRE(lsa.dataTag(adr1f_0xfabed001) == 0xfabed000);
	BOOST_REQUIRE(lsa.dataTag(adr3b_0xb8005385) == 0xb8005380);
}

BOOST_FIXTURE_TEST_CASE(readFromEmptyCache, EmptyCacheFixture)
{
	BOOST_REQUIRE(lsa.read(adr1a_0x06237817) == nullptr);
	BOOST_REQUIRE(lsa.read(adr3e_0x983724b0) == nullptr);
	BOOST_REQUIRE(lsa.read(adr1a_0x06237817) == nullptr);
}

BOOST_FIXTURE_TEST_CASE(readAndWriteOnce, EmptyCacheFixture)
{
	CacheLine *cl = nullptr;
	BOOST_REQUIRE(lsa.read(adr1a_0x06237817) == nullptr);
	BOOST_REQUIRE(lsa.write(adr1a_0x06237817) == nullptr);
	BOOST_REQUIRE((cl = lsa.read(adr1a_0x06237817)) != nullptr);
	BOOST_REQUIRE(cl->tag == lsa.dataTag(adr1a_0x06237817));
	BOOST_REQUIRE_EQUAL(table[lsa.locTag(adr1a_0x06237817)].size, 1);
	BOOST_REQUIRE_EQUAL(IS_ENTRY_INVALID(cl), false);
	BOOST_REQUIRE_EQUAL(IS_EVACUATED(cl), false);
}

BOOST_FIXTURE_TEST_CASE(multipleWriteAndRead, EmptyCacheFixture)
{
	BOOST_REQUIRE(lsa.write(adr2a_0x98325345) == nullptr);
	BOOST_REQUIRE(lsa.write(adr2d_0xb8725345) == nullptr);
	BOOST_REQUIRE(lsa.write(adr3a_0xa8115385) == nullptr);

	BOOST_REQUIRE(lsa.cacheTable[0].size == 0);
	BOOST_REQUIRE(lsa.cacheTable[0].set == nullptr);
	BOOST_REQUIRE(lsa.cacheTable[1].size == 2);
	BOOST_REQUIRE(lsa.cacheTable[1].set != nullptr);
	BOOST_REQUIRE(lsa.cacheTable[2].size == 1);
	BOOST_REQUIRE(lsa.cacheTable[2].set != nullptr);
	BOOST_REQUIRE(lsa.cacheTable[3].size == 0);
	BOOST_REQUIRE(lsa.cacheTable[3].set == nullptr);

	BOOST_REQUIRE(lsa.read(adr2a_0x98325345)->tag == lsa.dataTag(adr2a_0x98325345));
	BOOST_REQUIRE(lsa.read(adr2d_0xb8725345)->tag == lsa.dataTag(adr2d_0xb8725345));
	BOOST_REQUIRE(lsa.read(adr3a_0xa8115385)->tag == lsa.dataTag(adr3a_0xa8115385));
	BOOST_REQUIRE(lsa.read(adr1b_0x06737827) == nullptr);

	printCacheTable();
}

//int main()
//{
//	EmptyCacheFixture cache;
//	CacheLine *cl = nullptr;
//	cache.lsa.read(adr1a_0x06237817);
//	assert((cl = cache.lsa.write(adr1a_0x06237817)) == nullptr);
//	assert(cache.lsa.read(adr1a_0x06237817) == cl);
//	assert(cache.table[cache.lsa.locTag(adr1a_0x06237817)].size == 1);
//	return 0;
//}
