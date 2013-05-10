#include <iostream>
#include <iomanip>
using namespace std;

#include <lru.hpp>
#define private public //for testing!!
#include <set_accessor.hpp>

#define BOOST_TEST_MODULE SetAssociativityTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

//#define PRINT_TABLE

#ifdef PRINT_TABLE
#	define PRINTC() printCacheTable()
#else
#	define PRINTC()
#endif

Address adr1a_0x06237837 = 0x06237837, adr1b_0x06737837 = 0x06737837,
		adr1c_0x06a37837 = 0x06a37837, adr1d_0x06f37837 = 0x06f37837,
		adr1e_0x87600000 = 0x87600000, adr1f_0xfabed001 = 0xfabed001,

		adr2a_0x983253a5 = 0x983253a5, adr2b_0x987253a5 = 0x987253a5,
		adr2c_0xa87253a5 = 0xa87253a5, adr2d_0xb87253a5 = 0xb87253a5,

		adr3a_0xa8115345 = 0xa8115345, adr3b_0xb8005355 = 0xb8005355,
		adr3c_0xa8ff5365 = 0xa8ff5365, adr3d_0xb8345375 = 0xb8345375,

		adr4a_0x938753c5 = 0x938753c5, adr4b_0xffff53d5 = 0xffff53d5,
		adr4c_0x000053e5 = 0x000053e5, adr4d_0x0eca53f5 = 0x0eca53f5;

CacheConfig conf(4 * 4 * 64, LRU, SET_ASSOC, WRITE_BACK, 1, DATA_CACHE, 64, 4);

struct EmptyCacheFixture
{
	EmptyCacheFixture()
	: sa(conf)
	{
		sa.setCacheTable(&table);
	}

	CacheLine *writeAndUpdate(Address adr)
	{
		CacheLine * res = sa.write(adr);
		sa.updatePolicy(adr);
		return res;
	}

	CacheLine *readAndUpdate(Address adr)
	{
		CacheLine * res = sa.read(adr);
		sa.updatePolicy(adr);
		return res;
	}

	void printCacheTable()
	{
		CacheLine *set = NULL;
		cout << "\nNumber of sets in cache: " << table.size() << endl;
		for(auto itr = table.begin(); itr != table.end(); ++itr)
		{
			set = itr->second;
			cout << "[0x" << hex << setw(4) << setfill('0') << itr->first << "] :";
			for(size_t i = 0; i < conf.setSize; ++i)
			{
				if(!IS_CACHELINE_EMPTY(&set[i]))
				{
					cout << "{0x" << hex << setw(8) << setfill('0') << set[i].tag;
					cout << (IS_ENTRY_INVALID(&set[i]) ? ",INVALID" : "");
					cout << (IS_EVACUATED(&set[i]) ? ",EVACUATED" : "");
					cout << "} ";
				}
			}
			cout << endl;
		}
		cout << endl;
	}

	~EmptyCacheFixture()
	{
		PRINTC();
	}

	CacheTable table;
	SetAccessor<LeastRecentlyUsed<SET_ASSOC> >sa;
};

struct FullCacheFixture : EmptyCacheFixture
{
	FullCacheFixture()
	{
		writeAndUpdate(adr1a_0x06237837); writeAndUpdate(adr1b_0x06737837);
		writeAndUpdate(adr1c_0x06a37837); writeAndUpdate(adr1d_0x06f37837);

		writeAndUpdate(adr2a_0x983253a5); writeAndUpdate(adr2b_0x987253a5);
		writeAndUpdate(adr2c_0xa87253a5); writeAndUpdate(adr2d_0xb87253a5);

		writeAndUpdate(adr3a_0xa8115345); writeAndUpdate(adr3b_0xb8005355);
		writeAndUpdate(adr3c_0xa8ff5365); writeAndUpdate(adr3d_0xb8345375);

		writeAndUpdate(adr4a_0x938753c5); writeAndUpdate(adr4b_0xffff53d5);
		writeAndUpdate(adr4c_0x000053e5); writeAndUpdate(adr4d_0x0eca53f5);
	}
};

BOOST_FIXTURE_TEST_CASE(emptyCache, EmptyCacheFixture)
{
	BOOST_REQUIRE(sa.locTag(adr1a_0x06237837) == 0x00);
	BOOST_CHECK(sa.dataTag(adr1a_0x06237837) == 0x06237800);
	BOOST_CHECK(sa.locTag(adr2b_0x987253a5) == sa.locTag(adr2d_0xb87253a5));
	BOOST_CHECK(sa.locTag(adr4b_0xffff53d5) != sa.locTag(adr3c_0xa8ff5365));
	BOOST_CHECK(table.size() == 0);
	BOOST_CHECK(sa.read(adr1a_0x06237837) == NULL);
}

BOOST_FIXTURE_TEST_CASE(readWriteOnce, EmptyCacheFixture)
{
	BOOST_REQUIRE(writeAndUpdate(adr1a_0x06237837) == NULL);
	BOOST_REQUIRE_EQUAL(table.size(), 1);
	BOOST_REQUIRE(sa.read(adr1c_0x06a37837) == NULL);
}

BOOST_FIXTURE_TEST_CASE(fillOneColumn, EmptyCacheFixture)
{
	CacheLine *cl = NULL;

	BOOST_CHECK(writeAndUpdate(adr1a_0x06237837) == NULL);
	BOOST_CHECK(writeAndUpdate(adr2a_0x983253a5) == NULL);
	BOOST_CHECK(writeAndUpdate(adr3a_0xa8115345) == NULL);
	BOOST_CHECK(writeAndUpdate(adr4a_0x938753c5) == NULL);
	BOOST_CHECK(table.size() == 4);

	cl = sa.read(adr4a_0x938753c5);
	BOOST_CHECK(cl->tag == sa.dataTag(adr4a_0x938753c5));
	BOOST_CHECK(!IS_ENTRY_INVALID(cl));
	BOOST_CHECK(!IS_EVACUATED(cl));
	BOOST_CHECK(!IS_CACHELINE_EMPTY(cl));

	BOOST_CHECK(sa.read(adr1e_0x87600000) == NULL);
}

BOOST_FIXTURE_TEST_CASE(overwriteCacheLine, EmptyCacheFixture)
{
	CacheLine *cl = NULL;

	BOOST_CHECK(writeAndUpdate(adr1a_0x06237837) == NULL);
	BOOST_CHECK(writeAndUpdate(adr2a_0x983253a5) == NULL);
	BOOST_CHECK(writeAndUpdate(adr3a_0xa8115345) == NULL);
	BOOST_CHECK(writeAndUpdate(adr4a_0x938753c5) == NULL);
	BOOST_CHECK(writeAndUpdate(adr1c_0x06a37837) == NULL);

	cl = writeAndUpdate(adr1c_0x06a37837);
	BOOST_CHECK(IS_ENTRY_INVALID(cl));
	BOOST_CHECK(!IS_CACHELINE_EMPTY(cl));
	BOOST_CHECK(!IS_EVACUATED(cl));
}

BOOST_FIXTURE_TEST_CASE(writeToFullCache, FullCacheFixture)
{
	CacheLine *cl = NULL;

	cl = writeAndUpdate(adr1e_0x87600000);
	BOOST_CHECK(cl != NULL);
	BOOST_CHECK(cl->tag == sa.dataTag(adr1a_0x06237837));
	BOOST_CHECK(IS_EVACUATED(cl));
	BOOST_CHECK_EQUAL(table.size(), 4);
	cl = sa.read(adr1e_0x87600000);
	BOOST_CHECK(cl != NULL);
	BOOST_CHECK(!IS_EVACUATED(cl));
	BOOST_CHECK(!IS_CACHELINE_EMPTY(cl));
	BOOST_CHECK(!IS_ENTRY_INVALID(cl));

	cl = writeAndUpdate(adr1f_0xfabed001);
	BOOST_CHECK(cl != NULL);
	BOOST_CHECK(cl->tag == sa.dataTag(adr1b_0x06737837));
	BOOST_CHECK(IS_EVACUATED(cl));
	BOOST_CHECK_EQUAL(table.size(), 4);
	cl = sa.read(adr1f_0xfabed001);
	BOOST_CHECK(cl != NULL);
	BOOST_CHECK(!IS_EVACUATED(cl));
	BOOST_CHECK(!IS_CACHELINE_EMPTY(cl));
	BOOST_CHECK(!IS_ENTRY_INVALID(cl));
}
