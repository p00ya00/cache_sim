#ifndef SET_ACCESSOR_HPP_
#define SET_ACCESSOR_HPP_

#include "cache_config.hpp"
#include "exceptions.hpp"
#include <cmath>
using namespace std;
#define LOG2(N) (log(N) / log(2))

/**
 * accessor object for a set-associative cache.
 * the replacement policy should be provided as a template parameter.
 */
template <class ReplacementPolicyType>
class SetAccessor
{
public:
	SetAccessor(CacheConfig config)
	: cacheTable(nullptr), cacheConfig(config)
	{
		replPolicy = new ReplacementPolicyType(config.numberOfSets, config.setSize);
		int locTagSize  = ceil(LOG2(config.numberOfSets));
		int offsetTagSize = LOG2(config.cacheLineSize);
		dataTagMask = (~0) << (offsetTagSize);
		locTagMask = pow(2, locTagSize) - 1;
		locTagMask <<= offsetTagSize;
	}

	void setCacheTable(CacheTable *ct)
	{
		cacheTable = ct;
	}

	Tag locTag(Address adr)
	{
		return adr & locTagMask;
	}

	Tag dataTag(Address adr)
	{
		return adr & dataTagMask;
	}

	CacheLine *read(Address adr)
	{
		CacheTable::iterator itr = cacheTable->find(locTag(adr));
		if(itr != cacheTable->end())
		{
			CacheLine *set = itr->second;
			Tag dtag = dataTag(adr);
			for(short i = 0; i < cacheConfig.setSize; ++i)
				if(!IS_CACHELINE_EMPTY(&set[i]) && (set[i].tag == dtag))
					return &set[i];
		}
		return NULL;
	}

	CacheLine *write(Address adr)
	{
		Tag ltag = locTag(adr);
		Tag dtag = dataTag(adr);
		CacheTable::iterator itr = cacheTable->find(ltag);
		if(itr == cacheTable->end()) //if not in cache
		{
			if(cacheTable->size() < cacheConfig.numberOfSets) //cache not full
			{
				CacheLine *set = new CacheLine[cacheConfig.setSize];
				set[0].tag = dtag;
				for(short i = 1; i < cacheConfig.setSize; ++i)
					SET_CACHELINE_EMPTY(&set[i]);
				(*cacheTable)[ltag] = set;
				return NULL;
			}
			else //cache full but set not present, should not happen!
			{
				cout << "Invalid tag received for write!\n";
				throw InvalidTagException("Invalid tag received for write!");
			}
		}
		else //if in cache
		{
			CacheLine *set = itr->second;
			for(short i = 0; i < cacheConfig.setSize; ++i)
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
			Tag toReplaceTag = replPolicy->getEntry(ltag);
			CacheLine *replacedCl = NULL;
			for(short i = 0; i < cacheConfig.setSize; ++i)
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

	void updatePolicy(Address adr)
	{
		replPolicy->update(locTag(adr), dataTag(adr));
	}

	~SetAccessor()
	{
		if(replPolicy != nullptr)
			delete replPolicy;
	}

private:
	CacheTable *cacheTable;
	ReplacementPolicyType *replPolicy;
	CacheConfig cacheConfig;
	Tag locTagMask;
	Tag dataTagMask;
};

#endif /* SET_ACCESSOR_HPP_ */
