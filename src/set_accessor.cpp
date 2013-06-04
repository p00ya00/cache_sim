#include "set_accessor.hpp"

SetAccessor::SetAccessor(CacheConfig config)
: cacheTable(nullptr), cacheConfig(config)
{
	int locTagSize  = ceil(LOG2(config.numberOfSets));
	int offsetTagSize = LOG2(config.cacheLineSize);
	dataTagMask = (~0) << (offsetTagSize);
	locTagMask = pow(2, locTagSize) - 1;
	locTagMask <<= offsetTagSize;
}

void SetAccessor::setCacheTable(CacheTable ct)
{
	cacheTable = ct;
}

Tag SetAccessor::locTag(Address adr)
{
	return adr & locTagMask;
}

Tag SetAccessor::dataTag(Address adr)
{
	return adr & dataTagMask;
}

CacheLine *SetAccessor::read(Address adr)
{
	Tag ltag = locTag(adr);
	Tag dtag = dataTag(adr);

	// index cache table with the set tag
	CacheLineSet *cls = &cacheTable[ltag];
	for(CacheLineList *set = cls->set; set->next != nullptr; set = set->next)
	{
		if(set->cacheline->tag == dtag)
		{
			// move this cacheline to the end of the list
			DL_DELETE(cls->set, set);
			DL_APPEND(cls->set, set);
			return set->cacheline;
		}
	}

	return nullptr;
}

CacheLine *SetAccessor::write(Address adr)
{
	Tag ltag = locTag(adr);
	Tag dtag = dataTag(adr);

	// index cache table with the set tag
	CacheLineSet *cls = &cacheTable[ltag];
	// go through the set to find adr
	CacheLineList *entry = nullptr;
	for(entry = cls->set; entry->next != nullptr; entry = entry->next)
	{
		if(entry->cacheline->tag == dtag)
		{
			SET_ENTRY_INVALID(entry->cacheline);
			DL_DELETE(cls->set, entry);
			DL_APPEND(cls->set, entry);
			return entry->cacheline;
		}
	}
	// if not found save in cache table
	CacheLine *replacedCacheline = nullptr;
	// if set is full delete least recently used
	if(cls->size >= cacheConfig.setSize)
	{
		CacheLineList *toDelete = cls->set->prev;
		replacedCacheline = toDelete->cacheline;
		SET_EVACUATED(replacedCacheline);
		DL_DELETE(cls->set, toDelete);
		delete toDelete;
	}
	else
	{
		CacheLineList *newEntry = new CacheLineList;
		newEntry->cacheline = new CacheLine(dtag);
		DL_APPEND(cls->set, newEntry);
		++cls->size;
	}

	return replacedCacheline;
}
