#ifndef L2CACHE_HPP_
#define L2CACHE_HPP_

#include "set_accessor.hpp"
#include <unistd.h>
#include <iostream>
#include <iomanip>
using namespace std;

template <class NextLevel>
class L2Cache
{
public:
	L2Cache(CacheConfig c, NextLevel *next)
	: config(c), nextLevel(next), accessor(c), readRequests(8)
	, streamPrefetcher(8, &readRequests, config.cacheLineSize)
	, cleanLinesEvacuatedByDemand(0), dirtyLinesEvacuatedByDemand(0)
	, cleanLinesEvacuatedByPrefetcher(0), dirtyLinesEvacuatedByPrefetcher(0)
	, loadRequest(0), loadRequestHit(0), prefetchRequest(0), prefetchRequestMiss(0)
	{
		table = new CacheLineSet[c.numberOfSets];
		accessor.setCacheTable(table);
	}

	CacheOperationResult load(Address adr, bool prefetch = false)
	{
		CacheLine *cl = accessor.read(adr);
		CacheLine *modifiedOrEvacuated = nullptr;
		CacheOperationResult res;

		prefetch ? ++prefetchRequest : ++loadRequest;
		if(cl)
		{
			if(!prefetch) ++loadRequestHit;
			res = (CacheOperationResult)config.cacheLevel;
		}
		else
		{
			if(prefetch) ++prefetchRequestMiss;
			res = nextLevel->load(adr, prefetch);
			modifiedOrEvacuated = accessor.write(adr);
			//update performance counters
			if(modifiedOrEvacuated && IS_EVACUATED(modifiedOrEvacuated))
			{
				if(IS_ENTRY_INVALID(modifiedOrEvacuated))
					//evacuated dirty line
					prefetch ? ++dirtyLinesEvacuatedByPrefetcher : ++dirtyLinesEvacuatedByDemand;
				else
					//evacuated clean line
					prefetch ? ++cleanLinesEvacuatedByPrefetcher : ++cleanLinesEvacuatedByDemand;
			}

			if(modifiedOrEvacuated)
				handleWritePolicy(modifiedOrEvacuated);
		}
		//update should not be called after unsuccessful reads?
		if(!prefetch)
		{
			if(!cl) //new line was loaded into cache
			{
				//run spatial prefetcher
				// Address toFetch = spatialPrefetcher(adr);
				// load(toFetch, true);
				//cerr << "L2 spatial prefetcher loaded 0x" << hex << toFetch << dec << endl;
				// runStreamPrefetcher();
			}
		}

		return res;
	}

	CacheOperationResult store(Address adr)
	{
		CacheLine *cl = accessor.read(adr);
		CacheOperationResult res;
		CacheLine *modifiedOrEvacuated = nullptr;

		if(cl) //if already cached
		{
			modifiedOrEvacuated = accessor.write(adr);
			res = (CacheOperationResult)config.cacheLevel;
		}
		else
		{
			//load from next level
			res = nextLevel->load(adr);
			//store in cache
			modifiedOrEvacuated = accessor.write(adr);
			//write over the loaded entry
			accessor.write(adr);
		}

		//update performance counters
		if(modifiedOrEvacuated && IS_EVACUATED(modifiedOrEvacuated))
		{
			if(IS_ENTRY_INVALID(modifiedOrEvacuated))
				//evacuated dirty line
				++dirtyLinesEvacuatedByDemand;
			else
				//evacuated clean line
				++cleanLinesEvacuatedByDemand;
		}
		if(modifiedOrEvacuated)
			handleWritePolicy(modifiedOrEvacuated);

		return res;
	}

	void printCounters()
	{
		cerr << dec;
		cerr << "Clean lines evacuated by demand: " << cleanLinesEvacuatedByDemand << "\t\t";
		cerr << "Dirty lines evacuated by demand: " << dirtyLinesEvacuatedByDemand << endl;
		cerr << "Clean lines evacuated by prefetcher: " << cleanLinesEvacuatedByPrefetcher << "\t\t";
		cerr << "Dirty lines evacuated by prefetcher: " << dirtyLinesEvacuatedByPrefetcher << endl;
		cerr << "load requests: " << loadRequest << "\t\t";
		cerr << "load request hits: " << loadRequestHit << "\t\t";
		cerr << "prefetch request: " << prefetchRequest << endl;
		cerr << "prefetch request miss: " << prefetchRequestMiss << endl;
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

	Address spatialPrefetcher(Address adr)
	{
		//flip the 6th bit in the address, this way
		//the returned address points to the cache line required
		//to make a 128-byte chunk
		return adr ^ (1 << 6);
	}

	/*
	 * determines whether two addresses are in the same page
	 */
	bool inSamePage(Address a1, Address a2)
	{
		static unsigned long pageSize = sysconf(_SC_PAGESIZE);
		return ( (a1 & (-pageSize)) == (a2 & (-pageSize)) );
	}

	void runStreamPrefetcher()
	{
		void *toFetch, *seqStart;
		if(streamPrefetcher.ascendingPrefetch(&toFetch, &seqStart))
		{
			if(inSamePage((Address)toFetch, (Address)seqStart))
			{
				load((Address)toFetch, true);
				//cerr << "L2 ascedting stream prefetcher loaded 0x" << hex << toFetch << dec << endl;
			}
		}
		if(streamPrefetcher.descendingPrefetch(&toFetch, &seqStart))
		{
			if(inSamePage((Address)toFetch, (Address)seqStart))
			{
				load((Address)toFetch, true);
				//cerr << "L2 descedting stream prefetcher loaded 0x" << hex << toFetch << dec << endl;
			}
		}
	}

private:
	CacheTable table;
	CacheConfig config;
	NextLevel *nextLevel;
	SetAccessor accessor;
	AccessList readRequests;
	StreamPrefetcher streamPrefetcher;
	//performance counters
	size_t cleanLinesEvacuatedByDemand;
	size_t dirtyLinesEvacuatedByDemand;
	size_t cleanLinesEvacuatedByPrefetcher;
	size_t dirtyLinesEvacuatedByPrefetcher;
	size_t loadRequest;
	size_t loadRequestHit;
	size_t prefetchRequest;
	size_t prefetchRequestMiss;
};

#endif /* L2CACHE_HPP_ */
