#ifndef CACHE_DEF_HPP
#define CACHE_DEF_HPP

#include "uthash/utlist.h"
#include <boost/unordered_map.hpp>
#include <string>
using namespace std;

#define CACHE_LINE_SIZE 64 //Byte
#define MAX_CACHE_HISTORY 1024

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

//miss handler for the upper level cache
//typedef Byte *(*MissHandler)(Address adr);

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

string writePolicyToString(WritePolicy wp)
{
	switch(wp)
	{
	case WRITE_THROUGH:
		return "write-through";
	case WRITE_BACK:
		return "write-back";
	default:
		return "unknown";
	}
}

string replacementPolicyToString(ReplacementPolicy rp)
{
	switch(rp)
	{
	case LRU:
		return "LRU";
	case MRU:
		return "MRU";
	case RANDOM:
		return "random";
	default:
		return "unknown";
	}
}

string associativityToString(Associativity a)
{
	switch(a)
	{
	case SET_ASSOC:
		return "set-associative";
	case FULL_ASSOC:
		return "full-associative";
	default:
		return "unknown";
	}
}

enum CacheOperationResult
{
	MISS = 0, PREFETCH_MISS = 9,
	L1HIT = 1, // first level cache hit
	L2HIT, L3HIT, L4HIT  //...
};

struct CacheLine
{
	CacheLine(Tag t = 0, Flags f = 0)
	: tag(t), flags(f)
	{}

	Tag tag;
	Flags flags;
};

typedef boost::unordered_map<Tag, CacheLine *> CacheTable;

#endif /* CACHE_DEF_HPP */
