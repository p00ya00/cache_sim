#ifndef CACHE_HPP
#define CACHE_HPP

#include "cache_config.hpp"
#include "prefetcher.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>
using namespace std;

template <class Configuration>
class Cache
{
public:
	typedef Cache<Configuration> Type;

	Cache(Configuration config, typename Configuration::NextLevelType &next
			, PrefetchTrigger pt = NULL, int l = 1)
	: size(config.getCacheSize()), writePolicy(config.getWritePolicy())
	, nextLevel(next), prefetchTrigger(pt)
	, replPolicy(config.getNumberOfSets(), config.getSetSize())
	, assoc(config.getCacheSize(), table, replPolicy, config.getSetSize())
	, cacheLevel(l), accessCount(0), hitCount(0), dirtyBitMask(0x01)
	, prefetcherAccessCount(0), prefetcherHitCount(0)
	{
		assert(cacheLevel > 0);
	}

	CacheOperationResult load(Address adr, bool prefetching = false)
	{
		CacheLine *cl = assoc.read(adr);
		CacheLine *modifiedOrEvacuated = NULL;
		CacheOperationResult res;

		++accessCount;
		if(cl)
		{
			++hitCount;
			res = (CacheOperationResult)cacheLevel;
		}
		else
		{
			res = nextLevel.load(adr, prefetching);
			modifiedOrEvacuated = assoc.write(adr);
			if(writePolicy == WRITE_BACK)
			{
				if(modifiedOrEvacuated && IS_EVACUATED(modifiedOrEvacuated))
				{
					if(IS_ENTRY_INVALID(modifiedOrEvacuated))
						nextLevel.store(modifiedOrEvacuated->tag);
					delete modifiedOrEvacuated;
				}
			}
			else //writePolicy == WRITE_THROUGH
			{
				if(modifiedOrEvacuated)
				{
					if(IS_ENTRY_INVALID(modifiedOrEvacuated))
					{
						nextLevel.store(modifiedOrEvacuated->tag);
						SET_ENTRY_VALID(modifiedOrEvacuated);
					}
					if(IS_EVACUATED(modifiedOrEvacuated))
						delete modifiedOrEvacuated;
				}
			}
		}
		//update should not be called after unsuccessful reads?
		if(!prefetching)
		{
			replPolicy.update(assoc.locTag(adr), assoc.dataTag(adr));
			addToHistory(adr);
			if(prefetchTrigger)
					prefetch();
		}

		return res;
	}

	CacheOperationResult store(Address adr)
	{
		CacheLine *cl = assoc.read(adr);
		CacheOperationResult res;
		CacheLine *modifiedOrEvacuated = NULL;

		++accessCount;
		if(cl) //if already cached
		{
			++hitCount;
			modifiedOrEvacuated = assoc.write(adr);
			res = (CacheOperationResult)cacheLevel;
		}
		else
		{
			//load from next level
			res = nextLevel.load(adr);
			//store in cache
			modifiedOrEvacuated = assoc.write(adr);
			//write over the loaded entry
			assoc.write(adr);
		}
		if(writePolicy == WRITE_THROUGH)
		{
			if(modifiedOrEvacuated && IS_ENTRY_INVALID(modifiedOrEvacuated))
			{
				nextLevel.store(modifiedOrEvacuated->tag);
				SET_ENTRY_VALID(modifiedOrEvacuated);
			}
			if(modifiedOrEvacuated && IS_EVACUATED(modifiedOrEvacuated))
				delete modifiedOrEvacuated;
		}
		else //writePolicy == WRITE_BACK
		{
			if(modifiedOrEvacuated && IS_EVACUATED(modifiedOrEvacuated))
			{
				if(IS_ENTRY_INVALID(modifiedOrEvacuated))
					nextLevel.store(modifiedOrEvacuated->tag);
				delete modifiedOrEvacuated;
			}
		}
		replPolicy.update(assoc.locTag(adr), assoc.dataTag(adr));
		addToHistory(adr);

		return res;
	}

	void printStats()
	{
		cout << "Cache level: " << cacheLevel << endl;
		cout << "Associativity: " << associativityToString(assoc.getType());
		cout << ", Set size: " << assoc.getSetSize();
		cout << ", Number of sets: " << assoc.getNumberOfSets() << endl;
		cout << "Replacement policy: " << replacementPolicyToString(replPolicy.getReplacementPolicy()) << endl;
		cout << "Write policy: " << writePolicyToString(writePolicy) << endl;
		cout << "Cache size: " << size / 1024 << "KB\n";
		cout << "Access count: " << accessCount << ", Hit count: " << hitCount << endl;
	}

protected:

	void addToHistory(Address &adr)
	{
		AccessList *access = new AccessList;
		access->adr = adr;
		if(history.size >= MAX_CACHE_HISTORY)
		{
			AccessList *toDelete = history.getFirst();
			DL_DELETE(history.list, toDelete);
			delete toDelete;
			history.size--;
		}
		DL_APPEND(history.list, access);
		history.size++;
	}

	void prefetch()
	{
		Address *toFetch = NULL;
		prefetchTrigger(&history, &toFetch);
		if(toFetch)
		{
			load(*toFetch, true);
			delete toFetch;
		}
	}
private:
	CacheTable table;
	size_t size;
	WritePolicy writePolicy;
	typename Configuration::NextLevelType &nextLevel;
	PrefetchTrigger prefetchTrigger;
	CacheHistory history;
	typename Configuration::ReplacementPolicyType replPolicy;
	typename Configuration::AssociativityType assoc;
	int cacheLevel;
	int accessCount;
	int hitCount;
	int prefetcherAccessCount;
	int prefetcherHitCount;
	Flags dirtyBitMask;
};

#endif /* CACHE_HPP */
