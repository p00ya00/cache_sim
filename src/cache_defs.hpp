#ifndef CACHE_DEFS_HPP_
#define CACHE_DEFS_HPP_

#include <boost/unordered_map.hpp>
#include <cstddef>
#include <string>
using namespace std;
#include "uthash/utlist.h"

#define INVALID_ENTRY   0x01
#define EMPTY_CACHELINE 0x02
#define EVACUATED       0x04

#define IS_ENTRY_INVALID(cl) _IS_ENTRY_INVALID((cl))
#define IS_CACHELINE_EMPTY(cl) _IS_CACHELINE_EMPTY((cl))
#define IS_EVACUATED(cl) _IS_EVACUATED((cl))

#define SET_ENTRY_INVALID(cl) _SET_ENTRY_INVALID((cl))
#define SET_CACHELINE_EMPTY(cl) _SET_CACHELINE_EMPTY((cl))
#define SET_EVACUATED(cl) _SET_EVACUATED((cl))

#define SET_CACHELINE_FULL(cl) _SET_CACHELINE_FULL((cl))
#define SET_NOT_EVACUATED(cl) _SET_NOT_EVACUATED((cl))
#define SET_ENTRY_VALID(cl) _SET_ENTRY_VALID((cl))

#define _IS_ENTRY_INVALID(cl) ((cl->flags) & INVALID_ENTRY)
#define _IS_CACHELINE_EMPTY(cl) ((cl->flags) & EMPTY_CACHELINE)
#define _IS_EVACUATED(cl) ((cl->flags) & EVACUATED)

#define _SET_ENTRY_INVALID(cl) ((cl->flags) |= INVALID_ENTRY)
#define _SET_CACHELINE_EMPTY(cl) ((cl->flags) |= EMPTY_CACHELINE)
#define _SET_EVACUATED(cl) ((cl->flags) |= EVACUATED)

#define _SET_CACHELINE_FULL(cl) ((cl->flags) &= (~EMPTY_CACHELINE))
#define _SET_NOT_EVACUATED(cl) ((cl->flags) &= (~EVACUATED))
#define _SET_ENTRY_VALID(cl) ((cl->flags) &= (~INVALID_ENTRY))

typedef unsigned long long Tag;
typedef unsigned char Flags;
typedef unsigned long long Address;

struct CacheLine
{
	CacheLine(Tag t = 0, Flags f = 0)
	: tag(t), flags(f)
	{}

	Tag tag;
	Flags flags;
};

enum WritePolicy
{
	WRITE_THROUGH, WRITE_BACK
};

enum ReplacementPolicy
{
	LRU, MRU, RANDOM
};

enum Associativity
{
	SET_ASSOC, //includes direct-mapped as well
	FULL_ASSOC
};

enum CacheType
{
	DATA_CACHE,
	INSTRUCTION_CACHE,
	UNIFIED_CACHE
};

enum CacheOperationResult
{
	MISS = 0,
	L1HIT = 1, // first level cache hit
	L2HIT = 2,
	L3HIT = 3,
	PREFETCH_MISS = 4,
};

//history of recent cache accesses
typedef struct CacheAccess_
{
	Address adr;
	struct CacheAccess_ *prev;
	struct CacheAccess_ *next;
} CacheAccess;

typedef struct CacheLineList_
{
	CacheLine *cacheline;
	struct CacheLineSet_ *prev;
	struct CacheLineSet_ *next;
} CacheLineList;

typedef struct CacheLineSet_
{
	CacheLineList set;
	size_t size;
} CacheLineSet;

typedef CacheLineSet *CacheTable;

struct AccessList
{
	AccessList(size_t max = 128)
	: size(0), maxSize(max), list(NULL)
	{}

	CacheAccess *getFirst()
	{
		return list;
	}

	CacheAccess *getLast()
	{
		if(!list)
			return NULL;
		return list->prev;
	}

	void add(Address adr)
	{
		CacheAccess *access = new CacheAccess;
		access->adr = adr;
		if(size >= maxSize)
		{
			CacheAccess *toDelete = getFirst();
			DL_DELETE(list, toDelete);
			delete toDelete;
			size--;
		}
		DL_APPEND(list, access);
		size++;
	}

	size_t size;
	size_t maxSize;
	CacheAccess *list;
};

string writePolicyToString(WritePolicy wp);

string replacementPolicyToString(ReplacementPolicy rp);

string associativityToString(Associativity a);

#endif /* CACHE_DEFS_HPP_ */
