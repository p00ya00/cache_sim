#ifndef ASSOCIATIVITY_HPP
#define ASSOCIATIVITY_HPP

#include "cache_defs.hpp"
#include "exceptions.hpp"
#include <cmath>

#define LOG2(N) (log(N) / log(2))

template <class ReplacementPolicyType>
class FullAssociative
{
public:
	FullAssociative(size_t cacheSize, CacheTable &table, ReplacementPolicyType &rp, size_t ssize = 1)
	: maxSets(cacheSize / CACHE_LINE_SIZE), cacheTable(table), replPolicy(rp), setSize(ssize)
	, tagMask(~(CACHE_LINE_SIZE - 1))
	{}

	Tag locTag(Address adr)
	{
		return adr & tagMask;
	}

	Tag dataTag(Address adr)
	{
		return adr & tagMask;
	}

	Associativity getType()
	{
		return FULL_ASSOC;
	}

	size_t getSetSize()
	{
		return setSize;
	}

	size_t getNumberOfSets()
	{
		return maxSets;
	}

	//returns cache line containing the address
	CacheLine *read(Address adr)
	{
		CacheTable::iterator itr = cacheTable.find(locTag(adr));
		if(itr != cacheTable.end())
			return itr->second;
		return NULL;
	}

	//returns replaced cache line if any
	CacheLine *write(Address adr)
	{
		Tag ltag = locTag(adr);
		Tag dtag = dataTag(adr);
		CacheTable::iterator itr = cacheTable.find(ltag);
		if(itr == cacheTable.end()) //if not in cache
		{
			if(cacheTable.size() < maxSets) //cache not full
			{
				CacheLine *cl = new CacheLine(dtag);
				cacheTable[ltag] = cl;
				return NULL;
			}
			else //cache full
			{
				Tag toReplaceTag = replPolicy.getEntry(ltag);
				itr = cacheTable.find(toReplaceTag);
				CacheLine *toReplaceCl = itr->second;
				cacheTable.erase(itr);
				CacheLine *newCl = new CacheLine(dtag);
				cacheTable[ltag] = newCl;
				SET_EVACUATED(toReplaceCl);
				return toReplaceCl;
			}
		}
		else //if in cache
		{
			CacheLine *cl = itr->second;
			SET_ENTRY_INVALID(cl);
			return cl;
		}
	}

	static const Associativity type = FULL_ASSOC;

private:
	const size_t maxSets;
	CacheTable &cacheTable;
	ReplacementPolicyType &replPolicy;
	const size_t setSize;
	const Tag tagMask;
};

template <class ReplacementPolicyType>
class SetAssociative
{
public:
	SetAssociative(size_t cacheSize, CacheTable &table, ReplacementPolicyType &rp, size_t ssize)
	: numberOfSets(cacheSize / (CACHE_LINE_SIZE * ssize)), cacheTable(table), replPolicy(rp), setSize(ssize)
	{
		int locTagSize  = ceil(LOG2(numberOfSets));
		int offsetTagSize = LOG2(CACHE_LINE_SIZE);
		dataTagMask = (~0) << (offsetTagSize);
		locTagMask = pow(2, locTagSize) - 1;
		locTagMask <<= offsetTagSize;
	}

	Tag locTag(Address adr)
	{
		return adr & locTagMask;
	}

	Tag dataTag(Address adr)
	{
		return adr & dataTagMask;
	}

	Associativity getType()
	{
		return SET_ASSOC;
	}

	size_t getSetSize()
	{
		return setSize;
	}

	size_t getNumberOfSets()
	{
		return numberOfSets;
	}

	CacheLine *read(Address adr)
	{
		CacheTable::iterator itr = cacheTable.find(locTag(adr));
		if(itr != cacheTable.end())
		{
			CacheLine *set = itr->second;
			Tag dtag = dataTag(adr);
			for(size_t i = 0; i < setSize; ++i)
				if(!IS_CACHELINE_EMPTY(&set[i]) && (set[i].tag == dtag))
					return &set[i];
		}
		return NULL;
	}

	CacheLine *write(Address adr)
	{
		Tag ltag = locTag(adr);
		Tag dtag = dataTag(adr);
		CacheTable::iterator itr = cacheTable.find(ltag);
		if(itr == cacheTable.end()) //if not in cache
		{
			if(cacheTable.size() < numberOfSets) //cache not full
			{
				CacheLine *set = new CacheLine[setSize];
				set[0].tag = dtag;
				for(size_t i = 1; i < setSize; ++i)
					SET_CACHELINE_EMPTY(&set[i]);
				cacheTable[ltag] = set;
				return NULL;
			}
			else //cache full but set not present, should not happen!
				throw InvalidTagException("Invalid tag received for write!");
		}
		else //if in cache
		{
			CacheLine *set = itr->second;
			for(size_t i = 0; i < setSize; ++i)
			{
				//if place is free, we've been through filled
				//set members and they're not the entry we're looking for
				if(IS_CACHELINE_EMPTY(&set[i]))
				{
					set[i].tag = dtag;
					SET_CACHELINE_FULL(&set[i]);
					return NULL;
				}
				if(set[i].tag == dtag)
				{
					SET_ENTRY_INVALID(&set[i]);
					return &set[i];
				}
			}
			Tag toReplaceTag = replPolicy.getEntry(ltag);
			CacheLine *replacedCl = NULL;
			for(size_t i = 0; i < setSize; ++i)
				if(set[i].tag == toReplaceTag)
				{
					replacedCl = new CacheLine(set[i].tag, set[i].flags);
					SET_EVACUATED(replacedCl);
					set[i].tag = dtag;
					SET_ENTRY_VALID(&set[i]);
					break;
				}
			return replacedCl;
		}
	}

	static const Associativity type = SET_ASSOC;

private:
	const size_t numberOfSets;
	CacheTable &cacheTable;
	ReplacementPolicyType &replPolicy;
	const size_t setSize;
	Tag locTagMask;
	Tag dataTagMask;
	//static const Flags freeBitMask = 0x10;
};

#endif /* ASSOCIATIVITY_HPP */
