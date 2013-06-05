#ifndef L1ICACHE_HPP_
#define L1ICACHE_HPP_

#include "set_accessor.hpp"
#include "stride_prefetcher.hpp"
#include <iostream>
#include <iomanip>
using namespace std;

template <class NextLevel, class L1D>
class L1ICache
{
public:
	L1ICache(CacheConfig c, NextLevel *next, L1D *l1d)
	: config(c), nextLevel(next), l1dCache(l1d), accessor(c)
	, stridePrefetcher(config.cacheLineSize, 2048, 4), loadAccess(0), loadHit(0)
	{
		table = new CacheLineSet[c.numberOfSets];
		accessor.setCacheTable(table);
	}

	CacheOperationResult load(Address adr)
	{
		cout << "reading from accessor.\n";
		CacheLine *cl = accessor.read(adr);
		cout << "read from accessor.\n";
		CacheLine *modifiedOrEvacuated = nullptr;
		CacheOperationResult res;

		++loadAccess;
		if(cl) //Hit
		{
			++loadHit;
			res = (CacheOperationResult)config.cacheLevel;
		}
		else //Miss
		{
			res = nextLevel->load(adr);
			modifiedOrEvacuated = accessor.write(adr);
			if(modifiedOrEvacuated)
				handleWritePolicy(modifiedOrEvacuated);
		}

		return res;
	}

	void runPrefetcher(void *ip, Address loadOperand)
	{
		Address toFetch;

		if(stridePrefetcher.prefetch(ip, loadOperand, &toFetch))
		{
			l1dCache->load(toFetch, true);
			//cerr << "L1I stride prefetcher is loading 0x" << hex << toFetch << dec << endl;
		}
	}

	void printCounters()
	{
		cerr << dec;
		cerr << "Load access: " << loadAccess << "\t\t";
		cerr << "Load miss: " << loadAccess - loadHit << endl;
	}

private:
	void handleWritePolicy(CacheLine *modifiedCacheline)
	{
		if(config.writePolicy == WRITE_THROUGH)
		{
			if(IS_ENTRY_INVALID(modifiedCacheline))
			{
				nextLevel->store(modifiedCacheline->tag);
				SET_ENTRY_VALID(modifiedCacheline);
			}
			if(IS_EVACUATED(modifiedCacheline))
				delete modifiedCacheline;
		}
		else //writePolicy == WRITE_BACK
		{
			if(IS_EVACUATED(modifiedCacheline))
			{
				if(IS_ENTRY_INVALID(modifiedCacheline))
					nextLevel->store(modifiedCacheline->tag);
				delete modifiedCacheline;
			}
		}
	}

private:
	CacheTable table;
	CacheConfig config;
	NextLevel *nextLevel;
	L1D *l1dCache;
	SetAccessor accessor;
	//IP-based strided prefetcher
	StridePrefetcher stridePrefetcher;
	//performance counters
	size_t loadAccess;
	size_t loadHit;
};


#endif /* L1ICACHE_HPP_ */
