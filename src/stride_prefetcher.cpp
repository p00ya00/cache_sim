#include "stride_prefetcher.hpp"
#include <cmath> //for abs()
#include <iostream>
using namespace std;

StridePrefetcher::StridePrefetcher(size_t clSize, size_t maxStride, short minOcc, size_t tableSize)
: cacheLineSize(clSize), maxStrideSize(maxStride), minOccurence(minOcc)
, maxTableSize(tableSize)
{}

bool StridePrefetcher::prefetch(void *ip, Address loadedAdr, Address *toFetch)
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

void StridePrefetcher::deleteOneEntry()
{
	//cout << "deleteOneEntry()\n";
	//delete first entry with zero occurrence
	auto itr = table.begin();
	for(; itr != table.end(); ++itr)
	{
		if(itr->second->occurence == 0)
		{
			delete itr->second;
			table.erase(itr);
			return;
		}
	}
	//if couldn't find any entry to delete
	//just delete the first one
	itr == table.begin();
	delete itr->second;
	table.erase(itr);
}
