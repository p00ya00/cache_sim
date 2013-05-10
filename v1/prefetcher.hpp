#ifndef PREFETCHER_HPP
#define PREFETCHER_HPP

#define SEQ_PREFETCH_THRESHOLD 8

#include "cache_defs.hpp"
#include <unistd.h>
#include <iostream>
#include <cmath> //for abs()
using namespace std;

//history of recent addresses fetched into cache
typedef struct AccessList_
{
	Address adr;
	struct AccessList_ *prev;
	struct AccessList_ *next;
} AccessList;

struct CacheHistory
{
	CacheHistory()
	: size(0), list(NULL)
	{}

	AccessList *getFirst()
	{
		return list;
	}

	AccessList *getLast()
	{
		if(!list)
			return NULL;
		return list->prev;
	}

	size_t size;
	AccessList *list;
};

//based on cache history determines whether data
//should be prefetched into cache or not
typedef void (*PrefetchTrigger)(CacheHistory *history, Address **toFetch);
typedef bool (*AddressOrder)(const Address &, const Address &);

static bool inSamePage(Address a1, Address a2)
{
	static unsigned long pageSize = sysconf(_SC_PAGESIZE);
	return ( (a1 & (-pageSize)) == (a2 & (-pageSize)) );
}

static bool isAscending(const Address &a1, const Address &a2)
{
	return (a1 >= a2) && ((a1 - a2) < (2 * CACHE_LINE_SIZE));
}

static bool isDescending(const Address &a1, const Address &a2)
{
	return (a1 <= a2) && ((a2 - a1) < (2 * CACHE_LINE_SIZE));
}

static void sequentialAccess(CacheHistory *history, Address **toFetch, AddressOrder order)
{
	if(history->size < SEQ_PREFETCH_THRESHOLD)
	{
		*toFetch = NULL;
	}
	else
	{
		AccessList *access = history->getLast();
		AccessList *next;
		size_t n = 1;
		do
		{
			next = access;
			access = access->prev;
			if(!order(next->adr, access->adr))
			{
				*toFetch = NULL;
				return;
			}
			else if(++n >= SEQ_PREFETCH_THRESHOLD)
			{
				if(!inSamePage(access->adr, history->getLast()->adr + CACHE_LINE_SIZE))
					*toFetch = NULL;
				else
					*toFetch = new Address(history->getLast()->adr + CACHE_LINE_SIZE);
				return;
			}
		} while(access != history->list);
		*toFetch = NULL;
	}
}

void ascendingAccess(CacheHistory *history, Address **toFetch)
{
	sequentialAccess(history, toFetch, isAscending);
}

void descendingAccess(CacheHistory *history, Address **toFetch)
{
	sequentialAccess(history, toFetch, isDescending);
}

Address spatialPrefetcher(Address adr)
{
	//flip the 6th bit in the address, this way
	//the returned address points to the cache line required
	//to make a 128-byte chunk
	return adr ^ (1 << 6);
}

class StridePrefetcher
{
public:
	StridePrefetcher(size_t clSize, size_t maxStride, short minOcc, size_t tableSize = 256)
	: cacheLineSize(clSize), maxStrideSize(maxStride), minOccurence(minOcc)
	, maxTableSize(tableSize)
	{}

	/**
	 * check if a prefetch is possible
	 *
	 * @param ip instruction pointer of load operation
	 * @param loadedAdr address of memory operand of the load instruction
	 * @param toFetch contains address to fetch if return value is true
	 *
	 * @return true if a prefetch if possible with address to prefetch stored in
	 *         toFetch, false otherwise
	 */
	bool prefetch(void *ip, Address loadedAdr, Address *toFetch)
	{
		//if not already in table, create new entry for this load instruction
		auto itr = table.find(ip);
		if(itr == table.end())
		{
			if(table.size() >= maxTableSize)
				deleteOneEntry();
			PredictionTableEntry *entry =
					new PredictionTableEntry(ip, (void *)loadedAdr);
			table[ip] = entry;
			//since just inserted in table, no prefetching happens
			return false;
		}
		else //if already in table, update entry and check for prefetching
		{
			PredictionTableEntry *entry = itr->second;
			//if current and previous address fit in the same cacheline, ignore
			if(getCachelineAdr(loadedAdr) == getCachelineAdr((Address)(entry->prevAdr)))
			{
				entry->prevAdr = (void *)loadedAdr;
				return false;
			}
			int stride = getCachelineAdr(loadedAdr) - getCachelineAdr((Address)(entry->prevAdr));
			if(entry->stride == stride) //a stride happens again
			{
				entry->occurence++;
				if(entry->occurence == minOccurence)
				{
					entry->prevAdr = (void *)loadedAdr;
					*toFetch = loadedAdr + entry->stride;
					entry->occurence--;
					if(entry->prevPrefetchedAdr && (*toFetch == (Address)entry->prevPrefetchedAdr))
						return false;
					entry->prevPrefetchedAdr = (void *)*toFetch;
					return true;
				}
				entry->prevAdr = (void *)loadedAdr;
				return false;
			}
			else //different stride, clear previous occurrence counter of previous stride
			{
				if(abs(stride) <= (int)maxStrideSize)
				{
					entry->stride = stride;
					entry->occurence = 1;
				}
				else
				{
					entry->stride = 0;
					entry->occurence = 0;
				}
				entry->prevAdr = (void *)loadedAdr;
				return false;
			}
		}
		return false;
	}

private:
	/*
	 * make room for one new entry in the prediction table
	 */
	void deleteOneEntry()
	{
		//delete first entry with zero occurrence
		auto itr = table.begin();
		for(; itr != table.end(); ++itr)
		{
			if(itr->second->occurence == 0)
			{
				delete itr->second;
				table.erase(itr);
			}
		}
		//if couldn't find any entry to delete
		//just delete the first one
		if(itr == table.end())
		{
			itr == table.begin();
			delete itr->second;
			table.erase(itr);
		}
	}

	inline Address getCachelineAdr(Address adr)
	{
		return adr & ~(cacheLineSize - 1);
	}

	struct PredictionTableEntry
	{
		PredictionTableEntry(void *ipointer = nullptr, void *pAdr = nullptr)
		:ip(ipointer), prevAdr(pAdr), prevPrefetchedAdr(nullptr), stride(0)
		, occurence(0)
		{}

		void *ip;
		void *prevAdr;
		void *prevPrefetchedAdr;
		int stride; //negative: decreasing, positive: increasing, zero: not set
		short occurence;
	};

	typedef boost::unordered_map<void *, PredictionTableEntry *> PredictionTable;

	PredictionTable table;
	size_t cacheLineSize;
	size_t maxStrideSize;
	short minOccurence;
	const size_t maxTableSize;
};

#endif
