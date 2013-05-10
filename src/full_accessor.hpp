#ifndef FULL_ACCESSOR_HPP_
#define FULL_ACCESSOR_HPP_

#include "cache_config.hpp"
#include "exceptions.hpp"
#include <cmath>
using namespace std;
#define LOG2(N) (log(N) / log(2))


/**
 * accessor object for a full-associative cache.
 * the replacement policy should be provided as a template parameter.
 */
template <class ReplacementPolicyType>
class FullAccessor
{
public:
	FullAccessor(CacheConfig config)
	: cacheTable(nullptr), cacheConfig(config)
	, tagMask(~(config.cacheLineSize - 1))
	{
		replPolicy = new ReplacementPolicyType(config.numberOfSets);
	}

	void setCacheTable(CacheTable *ct)
	{
		cacheTable = ct;
	}

	Tag locTag(Address adr)
	{
		return adr & tagMask;
	}

	Tag dataTag(Address adr)
	{
		return adr & tagMask;
	}

	CacheLine *read(Address adr)
	{
		CacheTable::iterator itr = cacheTable->find(locTag(adr));
		if(itr != cacheTable->end())
			return itr->second;
		return nullptr;
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
				CacheLine *cl = new CacheLine(dtag);
				(*cacheTable)[ltag] = cl;
				return nullptr;
			}
			else //cache full
			{
				Tag toReplaceTag = replPolicy->getEntry(ltag);
				itr = cacheTable->find(toReplaceTag);
				CacheLine *toReplaceCl = itr->second;
				cacheTable->erase(itr);
				CacheLine *newCl = new CacheLine(dtag);
				(*cacheTable)[ltag] = newCl;
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

	void updatePolicy(Address adr)
	{
		replPolicy->update(locTag(adr), dataTag(adr));
	}

	~FullAccessor()
	{
		if(replPolicy != nullptr)
			delete replPolicy;
	}

private:
	CacheTable *cacheTable;
	ReplacementPolicyType *replPolicy;
	CacheConfig cacheConfig;
	Tag tagMask;
};

#endif /* FULL_ACCESSOR_HPP_ */
