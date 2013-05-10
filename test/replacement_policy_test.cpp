#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

#define BOOST_TEST_MODULE ReplacementPolicyTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#define private public //for testing!!
#include <lru.hpp>

#define LOG2(N) (log(N) / log(2))

struct FullLRUFixture
{
	FullLRUFixture()
	: cacheLineSize(64), numberOfSets(10), lruFull(numberOfSets)
	, tagMask(~(cacheLineSize - 1))
	{}

	void printLRUFullTable()
	{
		int i = 0;
		auto elm = lruFull.orderList;
		if(elm == NULL)
			cout << "empty LRU table!\n";
		else
		{
			cout << "LRU table size: " << lruFull.hmap.size() << endl;
			for(; elm != NULL; elm = elm->next)
				cout << i++ << ": " << hex << elm->tag << endl;
		}

	}

	Tag locTag(Address adr)
	{
		return adr & tagMask;
	}

	Tag dataTag(Address adr)
	{
		return adr & tagMask;
	}

	const size_t cacheLineSize;
	CacheTable table;
	size_t numberOfSets;
	LeastRecentlyUsed<FULL_ASSOC> lruFull;
	Tag tagMask;
};

struct SetLRUFixture
{
	SetLRUFixture()
	: cacheLineSize(64), numberOfSets(4), setSize(4)
	, lruSet(numberOfSets, setSize)
	{
		int locTagSize  = ceil(LOG2(numberOfSets));
		int offsetTagSize = LOG2(cacheLineSize);
		dataTagMask = (~0) << (offsetTagSize);
		locTagMask = pow(2, locTagSize) - 1;
		locTagMask <<= offsetTagSize;
	}

	void printLRUSetTable()
	{
		auto itr = lruSet.hmap.begin();
		if(itr == lruSet.hmap.end())
			cout << "empty LRU table!\n";
		else
		{
			cout << "LRU table size: " << lruSet.hmap.size() << endl;
			while(itr != lruSet.hmap.end())
			{
				cout << hex << itr->first << ": ";
				for(auto elm = itr->second; elm != NULL; elm = elm->next)
				{
					cout << "[" << elm->tag << "] ";
				}
				cout << endl;
				++itr;
			}
		}
	}

	Tag locTag(Address adr)
	{
		return adr & locTagMask;
	}

	Tag dataTag(Address adr)
	{
		return adr & dataTagMask;
	}

	const size_t cacheLineSize;
	short numberOfSets;
	short setSize;
	CacheTable table;
	LeastRecentlyUsed<SET_ASSOC> lruSet;
	Tag locTagMask;
	Tag dataTagMask;
};

BOOST_AUTO_TEST_SUITE(FullAssociativeLRU)

BOOST_FIXTURE_TEST_CASE(emptyLRU, FullLRUFixture)
{
	BOOST_REQUIRE_EQUAL(lruFull.hmap.size(), 0);
	try
	{
		lruFull.getEntry(0x0823734745);
		BOOST_REQUIRE_MESSAGE(false, "Must have thrown InvalidEvaquationReqException, but didn't!!");
	}
	catch(InvalidEvacuationReqException e)
	{}
}

BOOST_FIXTURE_TEST_CASE(notFullLRU, FullLRUFixture)
{
	lruFull.update(locTag(0x06237837), dataTag(0x06237837));
	lruFull.update(locTag(0x98325325), dataTag(0x98325325));
	lruFull.update(locTag(0x98734bae), dataTag(0x98734bae));
	lruFull.update(locTag(0xf7353fff), dataTag(0xf7353fff));
	lruFull.update(locTag(0x853ba230), dataTag(0x853ba230));
	lruFull.update(locTag(0x00000001), dataTag(0x00000001));
	lruFull.update(locTag(0xfffffffe), dataTag(0xfffffffe));
	lruFull.update(locTag(0x65481234), dataTag(0x65481234));
	lruFull.update(locTag(0x00444444), dataTag(0x00444444));
	BOOST_REQUIRE_EQUAL(lruFull.hmap.size(), 9);
	try
	{
		lruFull.getEntry(0x0823734745);
		BOOST_REQUIRE_MESSAGE(false, "Must have thrown InvalidEvaquationReqException, but didn't!!");
	}
	catch(InvalidEvacuationReqException e)
	{}
	lruFull.update(locTag(0x00000007), dataTag(0x00000007));
	BOOST_REQUIRE_EQUAL(lruFull.hmap.size(), 9);
}

BOOST_FIXTURE_TEST_CASE(fullLRU, FullLRUFixture)
{
	lruFull.update(locTag(0x06237837), dataTag(0x06237837));
	lruFull.update(locTag(0x98325325), dataTag(0x98325325));
	lruFull.update(locTag(0x98734bae), dataTag(0x98734bae));
	lruFull.update(locTag(0xf7353fff), dataTag(0xf7353fff));
	lruFull.update(locTag(0x853ba230), dataTag(0x853ba230));
	lruFull.update(locTag(0x00000001), dataTag(0x00000001));
	lruFull.update(locTag(0xfffffffe), dataTag(0xfffffffe));
	lruFull.update(locTag(0x65481234), dataTag(0x65481234));
	lruFull.update(locTag(0x00444444), dataTag(0x00444444));
	lruFull.update(locTag(0x00004411), dataTag(0x00004411));
	BOOST_REQUIRE_EQUAL(lruFull.hmap.size(), 10);
	Tag tag = lruFull.getEntry(0x05fde200);
	BOOST_REQUIRE(tag == locTag(0x06237837));

	lruFull.update(locTag(0x77777777), dataTag(0x77777777));
	BOOST_REQUIRE_EQUAL(lruFull.hmap.size(), 10);
	tag = lruFull.getEntry(0x05fde200);
	BOOST_REQUIRE(tag == locTag(0x98325325));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(SetAssociativeLRU)

BOOST_FIXTURE_TEST_CASE(emptyLRU, SetLRUFixture)
{
	BOOST_REQUIRE_EQUAL(lruSet.hmap.size(), 0);
	try
	{
		//must throw!
		lruSet.getEntry(0x0823734745);
		BOOST_REQUIRE_MESSAGE(false, "Must have thrown InvalidEvaquationReqException, but didn't!!");
	}
	catch(InvalidEvacuationReqException e)
	{}
}

BOOST_FIXTURE_TEST_CASE(halfFullLRU, SetLRUFixture)
{
	lruSet.update(locTag(0x06237837), dataTag(0x06237837));
	lruSet.update(locTag(0x983253a5), dataTag(0x983253a5));
	BOOST_REQUIRE_EQUAL(lruSet.hmap.size(), 2);
	try
	{
		//must throw!
		lruSet.getEntry(locTag(0x0823734745));
		BOOST_REQUIRE_MESSAGE(false, "Must have thrown InvalidEvaquationReqException, but didn't!!");
	}
	catch(InvalidEvacuationReqException e)
	{}
	lruSet.update(locTag(0x06737837), dataTag(0x06737837));
	lruSet.update(locTag(0x06a37837), dataTag(0x06a37837));
	lruSet.update(locTag(0x06f37837), dataTag(0x06f37837));
	lruSet.update(locTag(0x987253a5), dataTag(0x987253a5));
	BOOST_REQUIRE_EQUAL(lruSet.hmap.size(), 2);
	auto itr = lruSet.hmap.begin();
	BOOST_REQUIRE_EQUAL(lruSet.listSize(itr->second), 4);
	++itr;
	BOOST_REQUIRE_EQUAL(lruSet.listSize(itr->second), 2);
	try
	{
		lruSet.getEntry(locTag(0x0823734785));
		BOOST_REQUIRE_MESSAGE(false, "Must have thrown InvalidEvaquationReqException, but didn't!!");
	}
	catch(InvalidEvacuationReqException e)
	{}
}

BOOST_FIXTURE_TEST_CASE(fullLRU, SetLRUFixture)
{
	lruSet.update(locTag(0x06237837), dataTag(0x06237837));
	lruSet.update(locTag(0x983253a5), dataTag(0x983253a5));
	lruSet.update(locTag(0x06737837), dataTag(0x06737837));
	lruSet.update(locTag(0x06a37837), dataTag(0x06a37837));
	lruSet.update(locTag(0x06f37837), dataTag(0x06f37837));
	lruSet.update(locTag(0x987253a5), dataTag(0x987253a5));
	//must not throw
	BOOST_REQUIRE(lruSet.getEntry(locTag(0x0823734705)) == dataTag(0x06237837));
	lruSet.update(locTag(0xa87253a5), dataTag(0xa87253a5));
	lruSet.update(locTag(0xb87253a5), dataTag(0xb87253a5));
	lruSet.update(locTag(0xa8115345), dataTag(0xa8115345));
	lruSet.update(locTag(0xb8005355), dataTag(0xb8005355));
	lruSet.update(locTag(0xa8ff5365), dataTag(0xa8ff5365));
	lruSet.update(locTag(0xb8345375), dataTag(0xb8345375));
	lruSet.update(locTag(0x938753c5), dataTag(0x938753c5));
	lruSet.update(locTag(0xffff53d5), dataTag(0xffff53d5));
	lruSet.update(locTag(0x000053e5), dataTag(0x000053e5));
	lruSet.update(locTag(0x0eca53f5), dataTag(0x0eca53f5));
	BOOST_REQUIRE(lruSet.hmap.size() == 4);
	for(auto itr = lruSet.hmap.begin(); itr != lruSet.hmap.end(); ++itr)
		BOOST_REQUIRE(lruSet.listSize(itr->second) == 4);
	BOOST_REQUIRE(lruSet.hmap.size() == 4);
	BOOST_REQUIRE(lruSet.getEntry(locTag(0xaeee535f)) == 0xa8115340);
	lruSet.update(locTag(0xaeee535f), dataTag(0xaeee535f));
	BOOST_REQUIRE(lruSet.getEntry(locTag(0xabdc5360)) == 0xb8005340);
}

BOOST_AUTO_TEST_SUITE_END()
