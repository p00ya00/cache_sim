#ifndef SET_ACCESSOR_HPP_
#define SET_ACCESSOR_HPP_

#include "cache_config.hpp"
#include "exceptions.hpp"
#include <cmath>
using namespace std;
#define LOG2(N) (log(N) / log(2))

/**
 * accessor object for a LRU set-associative cache.
 */
class SetAccessor
{
public:
	SetAccessor(CacheConfig config);

	void setCacheTable(CacheTable ct);

	Tag locTag(Address adr);

	Tag dataTag(Address adr);

	CacheLine *read(Address adr);

	CacheLine *write(Address adr);

private:
	CacheTable cacheTable;
	CacheConfig cacheConfig;
	Tag locTagMask;
	Tag dataTagMask;
	int locTagSize;
	int offsetTagSize;
};

#endif /* SET_ACCESSOR_HPP_ */
