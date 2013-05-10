#ifndef CACHE_CONFIG_HPP
#define CACHE_CONFIG_HPP

#include "cache_defs.hpp"

template
<
	template <Associativity assoc> class ReplacementPolicyT,
	template <class ReplPolType> class AssociativityT,
	WritePolicy WP,
	class NextLevelT
>
class CacheConfiguration
{
public:
	typedef ReplacementPolicyT<AssociativityT<int>::type > ReplacementPolicyType;
	typedef AssociativityT<ReplacementPolicyType> AssociativityType;
	typedef NextLevelT NextLevelType;
	typedef CacheConfiguration<ReplacementPolicyT, AssociativityT, WP, NextLevelT> Type;

	CacheConfiguration(size_t cSize, size_t sSize = 1)
	: cacheSize(cSize), setSize(sSize), writePolicy(WP)
	, numberOfSets(cSize / (CACHE_LINE_SIZE * sSize))
	{}

	size_t getCacheSize()
	{
		return cacheSize;
	}

	size_t getSetSize()
	{
		return setSize;
	}

	size_t getNumberOfSets()
	{
		return numberOfSets;
	}

	WritePolicy getWritePolicy()
	{
		return writePolicy;
	}

private:
	size_t cacheSize;
	size_t setSize;
	WritePolicy writePolicy;
	size_t numberOfSets;
};

#endif /* CACHE_CONFIG_HPP */
