#ifndef PREFETCHER_HPP
#define PREFETCHER_HPP

#define SEQ_PREFETCH_THRESHOLD 8

#include "cache_defs.hpp"

class StreamPrefetcher
{
public:
	StreamPrefetcher(unsigned short threshold, AccessList *list, size_t clsize)
	: seqThreshold(threshold), recentLoads(list), cacheLineSize(clsize)
	{}

	bool ascendingPrefetch(void **toFetch, void **seqStart);

	bool descendingPrefetch(void **toFetch, void **seqStart);

private:
	typedef bool (StreamPrefetcher::*AddressOrder)(const Address &, const Address &);

	bool inSamePage(Address a1, Address a2);

	bool isAscending(const Address &a1, const Address &a2);

	bool isDescending(const Address &a1, const Address &a2);

	bool streamPrefetch(void **toFetch, void **seqStart, AddressOrder order);

private:
	unsigned short seqThreshold;
	AccessList *recentLoads;
	size_t cacheLineSize;
};

#endif
