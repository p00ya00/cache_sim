#ifndef MISS_HANDLER_HPP
#define MISS_HANDLER_HPP

#include "cache_defs.hpp"
#include <iostream>
using namespace std;

class MissHandler
{
public:
	MissHandler()
	{}

	void sendComp(size_t cycs)
	{
		//cout << "Computation " << cycs << " cycles" << endl;
	}

	void sendRW(Address adr, size_t size, size_t tid, char *rtnName,
				size_t timestamp, bool write, bool prefetching)
	{
		// cout << (write ? "Write " : "Read ") << size << " B in thread " << tid
		// 		<< " in function " << rtnName << ", Timestamp " << timestamp
		// 		<< (prefetching ? ", prefetched" : "") << endl;
	}
};

#endif /* MISS_HANDLER_HPP */
