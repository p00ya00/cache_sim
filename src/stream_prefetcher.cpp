#include "stream_prefetcher.hpp"
#include <unistd.h>
using namespace std;

bool StreamPrefetcher::inSamePage(Address a1, Address a2)
{
	static unsigned long pageSize = sysconf(_SC_PAGESIZE);
	return ( (a1 & (-pageSize)) == (a2 & (-pageSize)) );
}

bool StreamPrefetcher::isAscending(const Address &a1, const Address &a2)
{
	return (a1 >= a2) && ((a1 - a2) < (2 * cacheLineSize));
}

bool StreamPrefetcher::isDescending(const Address &a1, const Address &a2)
{
	return (a1 <= a2) && ((a2 - a1) < (2 * cacheLineSize));
}

bool StreamPrefetcher::streamPrefetch(void **toFetch, void **seqStart, AddressOrder order)
{
	if(recentLoads->size < seqThreshold)
	{
		*toFetch = *seqStart = nullptr;
		return false;
	}
	CacheAccess *access = recentLoads->getLast();
	CacheAccess *next;
	size_t n = 1;
	do
	{
		next = access;
		access = access->prev;
		if(!(this->*order)(next->adr, access->adr))
		{
			*toFetch = *seqStart = nullptr;
			return false;
		}
		else if(++n >= seqThreshold)
		{
			Address fetch = recentLoads->getLast()->adr + cacheLineSize;
			if(!inSamePage(access->adr, fetch))
			{
				*toFetch = *seqStart = nullptr;
				return false;
			}
			*toFetch = (void *)fetch;
			*seqStart = (void *)(access->adr);
			return true;
		}
	} while(access != recentLoads->list);

	*toFetch = *seqStart = nullptr;
	return false;
}

bool StreamPrefetcher::ascendingPrefetch(void **toFetch, void **seqStart)
{
	return streamPrefetch(toFetch, seqStart, &StreamPrefetcher::isAscending);
}

bool StreamPrefetcher::descendingPrefetch(void **toFetch, void **seqStart)
{
	return streamPrefetch(toFetch, seqStart, &StreamPrefetcher::isDescending);
}
/*
Address spatialPrefetcher(Address adr)
{
	//flip the 6th bit in the address, this way
	//the returned address points to the cache line required
	//to make a 128-byte chunk
	return adr ^ (1 << 6);
}
*/
