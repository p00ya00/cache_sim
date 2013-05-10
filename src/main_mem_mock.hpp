#ifndef MAINMEMMOCK_HPP
#define MAINMEMMOCK_HPP

#include "cache_defs.hpp"

class MainMemMock
{
public:
	MainMemMock()
	: storeCount(0), loadCount(0), prefetchCount(0)
	{}

	CacheOperationResult load(Address adr, bool prefetch = false)
	{
		prefetch ? ++prefetchCount : ++loadCount;
		return (prefetch ? PREFETCH_MISS : MISS);
	}

	CacheOperationResult store(Address adr)
	{
		++storeCount;
		return MISS;
	}

	void printStats()
	{
		cerr << "Main memory" << dec << endl;
		cerr << "Access count: " << loadCount + storeCount + prefetchCount << endl;
		cerr << "loads: " << loadCount << endl;
		cerr << "stores: " << storeCount << endl;
		cerr << "prefetches: " << prefetchCount << endl;
	}

private:
	size_t storeCount;
	size_t loadCount;
	size_t prefetchCount;
};

#endif /* MAINMEMMOCK_HPP_ */
