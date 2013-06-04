#ifndef L1DCACHE_HPP_
#define L1DCACHE_HPP_

#include "set_accessor.hpp"
#include "stream_prefetcher.hpp"
#include <iostream>
#include <iomanip>
using namespace std;

template <class NextLevel>
class L1DCache
{
public:
	L1DCache(CacheConfig c, NextLevel *next)
	: config(c), nextLevel(next), accessor(c)
	, streamPrefetcher(8, &recentlyLoadedLines, config.cacheLineSize)
	, loadAccess(0), loadHit(0), storeAccess(0), storeHit(0)
	, prefetchAccess(0), prefetchHit(0), loadedCachelines(0)
	, modifiedLinesEvacuated(0), writebacksToL2(0)
	{
		table = new CacheLineSet[c.numberOfSets];
		accessor.setCacheTable(table);
	}

	/*
	 * @param prefetch whether this load request is from a prefetcher
	 */
	CacheOperationResult load(Address adr, bool prefetch = false)
	{
		CacheLine *cl = accessor.read(adr);
		CacheLine *modifiedOrEvacuated = nullptr;
		CacheOperationResult res;

		prefetch ? ++prefetchAccess : ++loadAccess;
		if(cl) //Hit
		{
			prefetch ? ++prefetchHit : ++loadHit;
			res = (CacheOperationResult)config.cacheLevel;
		}
		else  //Miss
		{
			++loadedCachelines;
			res = nextLevel->load(adr, prefetch);
			modifiedOrEvacuated = accessor.write(adr);
			if(!prefetch)
				recentlyLoadedLines.add(adr);
			if(modifiedOrEvacuated)
				handleWritePolicy(modifiedOrEvacuated);
		}
		if(modifiedOrEvacuated && IS_ENTRY_INVALID(modifiedOrEvacuated) && IS_EVACUATED(modifiedOrEvacuated))
			++modifiedLinesEvacuated;

		return res;
	}

	CacheOperationResult store(Address adr)
	{
		CacheLine *cl = accessor.read(adr);
		CacheOperationResult res;
		CacheLine *modifiedOrEvacuated = nullptr;

		++storeAccess;
		if(cl) //if already cached
		{
			++storeHit;
			modifiedOrEvacuated = accessor.write(adr);
			res = (CacheOperationResult)config.cacheLevel;
		}
		else
		{
			//load from next level
			res = nextLevel->load(adr);
			//store in cache
			modifiedOrEvacuated = accessor.write(adr);
			++loadedCachelines;
			//write over the loaded entry
			accessor.write(adr);
		}
		if(modifiedOrEvacuated)
			handleWritePolicy(modifiedOrEvacuated);

		if(modifiedOrEvacuated && IS_ENTRY_INVALID(modifiedOrEvacuated) && IS_EVACUATED(modifiedOrEvacuated))
			++modifiedLinesEvacuated;

		return res;
	}

	void printCounters()
	{
		cerr << dec;
		cerr << "Load access: " << loadAccess << "\t\t";
		cerr << "Load miss: " << loadAccess - loadHit << "\t\t\t";
		cerr << "Store access: " << storeAccess << endl;
		cerr << "Store miss: " << storeAccess - storeHit << "\t\t";
		cerr << "Prefetch access: " << prefetchAccess << "\t\t";
		cerr << "Prefetch miss: " << prefetchAccess - prefetchHit << endl;
		cerr << "Loaded cachelines: " << loadedCachelines << "\t\t";
		cerr << "Modified lines evacuated: " << modifiedLinesEvacuated << "\t";
		cerr << "Writebacks to L2: " << writebacksToL2 << endl;
	}

	unsigned long long getLoadAccess()
	{
		return loadAccess;
	}

private:
	void runPrefetcher()
	{
		//according to the Intel manual prefetcher should be called only when the cache is write-back
		if(config.writePolicy == WRITE_THROUGH)
			return;
		void *toFetch, *seqStart;
		if(streamPrefetcher.ascendingPrefetch(&toFetch, &seqStart))
		{
			load((Address)toFetch, true);
			// cerr << "L1D stream prefetcher loaded 0x" << hex << toFetch << dec << endl;
		}
	}

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
				{
					nextLevel->store(modifiedCacheline->tag);
					++writebacksToL2;
				}
				delete modifiedCacheline;
			}
		}
	}

private:
	CacheTable table;
	CacheConfig config;
	NextLevel *nextLevel;
	SetAccessor accessor;
	AccessList recentlyLoadedLines;
	//ascending stream prefetcher
	StreamPrefetcher streamPrefetcher;
	//performance counters
	size_t loadAccess;
	size_t loadHit;
	size_t storeAccess;
	size_t storeHit;
	size_t prefetchAccess;
	size_t prefetchHit;
	size_t loadedCachelines;
	size_t modifiedLinesEvacuated;
	size_t writebacksToL2;
};


#endif /* L1DCACHE_HPP_ */
