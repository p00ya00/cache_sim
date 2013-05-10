#ifndef MAINMEMMOCK_HPP
#define MAINMEMMOCK_HPP

#include "cache_defs.hpp"

class MainMemMock
{
public:
	MainMemMock()
	: storeCount(0), loadCount(0)
	{}

	CacheOperationResult load(Address adr, bool prefetching = false)
	{
		++loadCount;
		return (prefetching ? PREFETCH_MISS : MISS);
	}

	CacheOperationResult store(Address adr)
	{
		++storeCount;
		return MISS;
	}

	void printStats()
	{
		cout << "Main memory" << endl;
		cout << "Access count: " << loadCount + storeCount << endl;
	}

private:
	size_t storeCount;
	size_t loadCount;
};

#endif /* MAINMEMMOCK_HPP_ */
