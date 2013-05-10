#include <iostream>
#include <iomanip>
using namespace std;

#define BOOST_TEST_MODULE FullAssociativityTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <lru.hpp>
#define private public //for testing!!
#include <full_accessor.hpp>

CacheConfig conf(10 * 64, LRU, FULL_ASSOC, WRITE_BACK, 1, DATA_CACHE, 64);

struct FullLRUFixture
{
	FullLRUFixture()
	: fa(conf)
	{
		fa.setCacheTable(&table);
	}

	void printTable()
	{
		cout << "\nNumber of entries in cache: " << table.size() << endl;
		for(auto itr = table.begin(); itr != table.end(); ++itr)
		{
			cout << "[0x" << hex << setw(8) << setfill('0') << itr->first << "] ";
			cout << (IS_ENTRY_INVALID(itr->second) ? "INVALID " : "");
			cout << (IS_EVACUATED(itr->second) ? "EVACUATED " : "");
			cout << endl;
		}
	}

	CacheLine *writeAndUpdate(Address adr)
	{
		CacheLine *res = fa.write(adr);
		fa.updatePolicy(adr);
		return res;
	}

	CacheLine *readAndUpdate(Address adr)
	{
		CacheLine * res = fa.read(adr);
		fa.updatePolicy(adr);
		return res;
	}

	void fillTable()
	{
		// fill the table
		BOOST_REQUIRE(writeAndUpdate(adr1_0x005381aa) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr2_0xff001238) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr3_0x369146aa) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr4_0x987654ff) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr5_0xcccccccc) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr6_0x99990034) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr7_0x00000000) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr8_0xfdeac982) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr9_0x99887766) == NULL);
		BOOST_REQUIRE(writeAndUpdate(adr10_0x11111111) == NULL);
	}

	CacheTable table;
	FullAccessor<LeastRecentlyUsed<FULL_ASSOC> > fa;

	static const Address adr1_0x005381aa = 0x005381aa, adr2_0xff001238 = 0xff001238,
				adr3_0x369146aa = 0x369146aa, adr4_0x987654ff = 0x987654ff,
				adr5_0xcccccccc = 0xcccccccc, adr6_0x99990034 = 0x99990034,
				adr7_0x00000000 = 0x00000000, adr8_0xfdeac982 = 0xfdeac982,
				adr9_0x99887766 = 0x99887766, adr10_0x11111111 = 0x11111111,
				adr11_0x005381bf = 0x005381bf, adr12_0xbbbbbbbb = 0xbbbbbbbb,
				adr13_0x13660507 = 0x13660507, adr14_0x19870729 = 0x19870729;
};

BOOST_AUTO_TEST_SUITE(fullAssociativity)

BOOST_FIXTURE_TEST_CASE(emptyTable, FullLRUFixture)
{
	BOOST_CHECK(fa.locTag(adr1_0x005381aa) == 0x00538180);
	BOOST_CHECK(fa.dataTag(adr2_0xff001238) == 0xff001200);
//	BOOST_CHECK_EQUAL(fa.getSetSize(), 1);
//	BOOST_CHECK_EQUAL(fa.getNumberOfSets(), 10);
	BOOST_CHECK(fa.read(adr1_0x005381aa) == NULL);
}

BOOST_FIXTURE_TEST_CASE(singleWrite, FullLRUFixture)
{
	BOOST_CHECK(writeAndUpdate(adr1_0x005381aa) == NULL);
	CacheLine *clRead = readAndUpdate(adr1_0x005381aa);
	BOOST_CHECK(clRead->tag == fa.locTag(adr1_0x005381aa));
	BOOST_CHECK(!IS_EVACUATED(clRead) && !IS_ENTRY_INVALID(clRead));
	BOOST_CHECK_EQUAL(table.size(), 1);
}

BOOST_FIXTURE_TEST_CASE(overwriteEntry, FullLRUFixture)
{
	BOOST_CHECK(writeAndUpdate(adr2_0xff001238) == NULL);
	CacheLine *clRead = readAndUpdate(adr2_0xff001238);
	CacheLine *clWritten = writeAndUpdate(adr2_0xff001238);
	BOOST_REQUIRE(clWritten != NULL);
	BOOST_REQUIRE(clRead == clWritten);
	BOOST_REQUIRE(!IS_EVACUATED(clWritten));
	BOOST_REQUIRE(IS_ENTRY_INVALID(clWritten));
}

BOOST_FIXTURE_TEST_CASE(fullTableReadAndOverwrite, FullLRUFixture)
{
	fillTable();

	CacheLine *clRead = readAndUpdate(adr5_0xcccccccc);
	BOOST_REQUIRE(clRead->tag == fa.locTag(adr5_0xcccccccc));
	BOOST_REQUIRE(!IS_EVACUATED(clRead) && !IS_ENTRY_INVALID(clRead));

	//overwrite cacheline with different adr but still in cacheline range
	CacheLine *clWritten = writeAndUpdate(adr11_0x005381bf);
	clRead = readAndUpdate(adr1_0x005381aa);
	BOOST_REQUIRE(clWritten != NULL);
	BOOST_REQUIRE(clRead == clWritten);
	BOOST_REQUIRE(!IS_EVACUATED(clWritten));
	BOOST_REQUIRE(IS_ENTRY_INVALID(clWritten));
	BOOST_REQUIRE(table.size() == 10);
}

BOOST_FIXTURE_TEST_CASE(fullTableReadAndReplace, FullLRUFixture)
{
	fillTable();

	CacheLine *replaced = writeAndUpdate(adr12_0xbbbbbbbb);
	BOOST_REQUIRE(replaced != NULL);
	//by now cacheline from adr1 should be the LRU
	BOOST_REQUIRE(replaced->tag == fa.locTag(adr1_0x005381aa));
	BOOST_REQUIRE(IS_EVACUATED(replaced));
	BOOST_REQUIRE_EQUAL(table.size(), 10);
	BOOST_REQUIRE(fa.read(adr1_0x005381aa) == NULL);

	replaced = writeAndUpdate(adr13_0x13660507);
	BOOST_REQUIRE(replaced != NULL);
	//by now cacheline from adr2 should be the LRU
	BOOST_REQUIRE(replaced->tag == fa.locTag(adr2_0xff001238));
	BOOST_REQUIRE(IS_EVACUATED(replaced));
	BOOST_REQUIRE(table.size() == 10);
	BOOST_REQUIRE(fa.read(adr2_0xff001238) == NULL);
}

BOOST_AUTO_TEST_SUITE_END()
