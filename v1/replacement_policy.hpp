#ifndef REPLACEMENT_POLICY_HPP
#define REPLACEMENT_POLICY_HPP

#include "cache_defs.hpp"
#include "exceptions.hpp"

template <Associativity assocType>
class LeastRecentlyUsed
{};

template <>
class LeastRecentlyUsed <FULL_ASSOC>
{
public:
	LeastRecentlyUsed(size_t numOfSets, size_t sSize = 1)
	: orderList(NULL), numberOfSets(numOfSets), setSize(sSize)
	{}

	Tag getEntry(Tag t)
	{
		if(orderList == NULL || hmap.size() < numberOfSets)
			throw InvalidEvacuationReqException("Bad evacuation request! Cache is empty!!");
		return orderList->tag;
	}

	void update(Tag locTag, Tag dataTag)
	{
		HashMap::iterator itr = hmap.find(locTag);
		if(itr == hmap.end()) //if entry is not in the table
		{
			if(hmap.size() < numberOfSets) //if table is not full add a new entry
			{
				ListElement *elm = new ListElement;
				elm->tag = dataTag;
				hmap[locTag] = elm;
				DL_APPEND(orderList, elm);
			}
			else //if table is full, delete an entry (LRU)
			{
				HashMap::iterator toDelete = hmap.find(orderList->tag);
				DL_DELETE(orderList, toDelete->second);
				delete toDelete->second;
				hmap.erase(toDelete);

				ListElement *elm = new ListElement;
				elm->tag = dataTag;
				hmap[locTag] = elm;
				DL_APPEND(orderList, elm);
			}
		}
		else //entry exits in table
		{
			DL_DELETE(orderList, itr->second);
			DL_APPEND(orderList, itr->second);
			//ListElement *elm = new ListElement;
			//elm->tag = itr->first;
			//DL_APPEND(orderList, elm);
			//itr->second = elm;
		}
	}

	ReplacementPolicy getReplacementPolicy()
	{
		return LRU;
	}

private:
	typedef struct ListElement_
	{
		Tag tag;
		struct ListElement_ *prev;
		struct ListElement_ *next;
	} ListElement;
	typedef boost::unordered_map<Tag, ListElement *> HashMap;

	HashMap hmap;
	ListElement *orderList;
	size_t numberOfSets;
	size_t setSize;
};

template <>
class LeastRecentlyUsed <SET_ASSOC>
{
public:
	LeastRecentlyUsed(size_t numOfSets, size_t sSize = 1)
	: numberOfSets(numOfSets), setSize(sSize)
	{}

	Tag getEntry(Tag t)
	{
		HashMap::iterator itr = hmap.find(t);
		if(itr == hmap.end() || listSize(itr->second) < setSize)
			throw InvalidEvacuationReqException("Bad evacuation request! Cache not full yet!!");
		return itr->second->tag;
	}

	void update(Tag locTag, Tag dataTag)
	{
		HashMap::iterator itr = hmap.find(locTag);
		if(itr == hmap.end()) //if entry is not in the table
		{
			if(hmap.size() < numberOfSets) //if table is not full add a new entry
			{
				ListElement *list = NULL;
				ListElement *elm = new ListElement;
				elm->tag = dataTag;
				DL_APPEND(list, elm);
				hmap[locTag] = list;
			}
			else //if table is full, throw
				throw InvalidTagException("Invalid address received in LRU (set-associate)!");
		}
		else //row exits in table
		{
			if(listSize(itr->second) >= setSize) //set is full, delete least recently used address
			{
				ListElement *toDelete = itr->second;
				DL_DELETE(itr->second, itr->second);
				delete toDelete;
			}

			ListElement *elm = new ListElement;
			elm->tag = dataTag;
			DL_APPEND(itr->second, elm);
		}
	}

	ReplacementPolicy getReplacementPolicy()
	{
		return LRU;
	}

private:
	typedef struct ListElement_
	{
		Tag tag;
		struct ListElement_ *prev;
		struct ListElement_ *next;
	} ListElement;
	typedef boost::unordered_map<Tag, ListElement *> HashMap;

	size_t listSize(ListElement *list)
	{
		size_t size = 0;
		while(list != NULL)
		{
			++size;
			list = list->next;
		}
		return size;
	}

	HashMap hmap;
	size_t numberOfSets;
	size_t setSize;
};

#endif //REPLACEMENT_POLICY_HPP
