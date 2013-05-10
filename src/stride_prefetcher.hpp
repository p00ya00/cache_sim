#ifndef STRIDE_PREFETCHER_HPP_
#define STRIDE_PREFETCHER_HPP_

#include "cache_defs.hpp"

class StridePrefetcher
{
public:
	StridePrefetcher(size_t clSize, size_t maxStride, short minOcc, size_t tableSize = 256);

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
	bool prefetch(void *ip, Address loadedAdr, Address *toFetch);

private:
	/*
	 * make room for one new entry in the prediction table
	 */
	void deleteOneEntry();

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

#endif /* STRIDE_PREFETCHER_HPP_ */
