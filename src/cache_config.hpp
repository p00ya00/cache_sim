#ifndef CACHE_CONFIG_HPP_
#define CACHE_CONFIG_HPP_

#include "cache_defs.hpp"

struct CacheConfig
{
	CacheConfig(size_t csize, ReplacementPolicy rp, Associativity a, WritePolicy wp,
			short level, CacheType ct, size_t clsize, short ssize = 1)
	: cacheSize(csize), setSize(ssize), replPolicy(rp), assoc(a), writePolicy(wp)
	, cacheLevel(level), cacheType(ct), cacheLineSize(clsize)
	{
		numberOfSets = cacheSize / (cacheLineSize * setSize);
	}

	size_t cacheSize;
	size_t numberOfSets;
	short setSize;
	ReplacementPolicy replPolicy;
	Associativity assoc;
	WritePolicy writePolicy;
	short cacheLevel;
	CacheType cacheType;
	size_t cacheLineSize;
};

#endif /* CACHE_CONFIG_HPP_ */
