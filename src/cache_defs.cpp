#include "cache_defs.hpp"

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
