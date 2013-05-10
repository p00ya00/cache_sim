#ifndef INSTRCOST_HPP
#define INSTRCOST_HPP

#define COST_FILE_COMMENT "#"

#include "cache_defs.hpp"
#include "pin.H"
using namespace LEVEL_CORE;

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include <boost/algorithm/string.hpp>
using namespace boost::algorithm;

class InstrCostTable
{
public:
	InstrCostTable(string filePath = "")
	{
		if(filePath.empty())
		{
			cerr << "Instruction cost file not found. Using default values!\n";
			fillTableWithDefaultValues();
		}
		else
		{
			ifstream costFile(filePath.c_str());
			if(costFile.is_open())
			{
				fillTableFromFile(costFile);
				costFile.close();
			}
			else
				fillTableWithDefaultValues();
		}
	}

	size_t lookup(INS &ins)
	{
		CATEGORY category = static_cast<CATEGORY>(INS_Category(ins));
		CostTable::iterator itr = table.find(category);
		if(itr != table.end())
			return itr->second;
		return 0;
	}

	size_t lookup(CacheOperationResult cacheOp)
	{
		if(cacheOp == MISS)
			return cacheMissCost;
		else
			return cacheHitCost;
	}
#if false
	void printTable()
	{
		CostTable::iterator itr;
		for(itr = table.begin(); itr != table.end(); ++itr)
			cout << CATEGORY_StringShort(itr->first) << ", " << itr->second << endl;
		cout << "CACHE_HIT\t" << cacheHitCost << endl;
		cout << "CACHE_MISS\t" << cacheMissCost << endl;
	}
#endif
private:
	typedef boost::unordered_map<CATEGORY, size_t> CostTable;
	CostTable table;
	size_t cacheHitCost;
	size_t cacheMissCost;

private:
	void fillTableWithDefaultValues()
	{
		table[XED_CATEGORY_3DNOW] = 5;			table[XED_CATEGORY_AES] = 3;
		table[XED_CATEGORY_AVX] = 5;			table[XED_CATEGORY_AVX2] = 6;
		table[XED_CATEGORY_AVX2GATHER] = 1;		table[XED_CATEGORY_BINARY] = 8;
		table[XED_CATEGORY_BITBYTE] = 8;		table[XED_CATEGORY_BMI1] = 3;
		table[XED_CATEGORY_BMI2] = 6;			table[XED_CATEGORY_BROADCAST] = 7;
		table[XED_CATEGORY_CALL] = 5;			table[XED_CATEGORY_CMOV] = 3;
		table[XED_CATEGORY_COND_BR] = 9;		table[XED_CATEGORY_CONVERT] = 2;
		table[XED_CATEGORY_DATAXFER] = 8;		table[XED_CATEGORY_DECIMAL] = 8;
		table[XED_CATEGORY_FCMOV] = 7;			table[XED_CATEGORY_FLAGOP] = 4;
		table[XED_CATEGORY_INTERRUPT] = 7;		table[XED_CATEGORY_INVALID] = 7;
		table[XED_CATEGORY_IO] = 6;				table[XED_CATEGORY_IOSTRINGOP] = 5;
		table[XED_CATEGORY_LOGICAL] = 9;		//////
		table[XED_CATEGORY_LZCNT] = 4;			table[XED_CATEGORY_MISC] = 4;
		table[XED_CATEGORY_MMX] = 3;			table[XED_CATEGORY_NOP] = 7;
		table[XED_CATEGORY_PCLMULQDQ] = 4;		table[XED_CATEGORY_POP] = 3;
		table[XED_CATEGORY_PREFETCH] = 5;		table[XED_CATEGORY_PUSH] = 7;
		table[XED_CATEGORY_RDRAND] = 6;			table[XED_CATEGORY_RDWRFSGS] = 4;
		table[XED_CATEGORY_RET] = 7;			table[XED_CATEGORY_ROTATE] = 6;
		table[XED_CATEGORY_SEGOP] = 8;			table[XED_CATEGORY_SEMAPHORE] = 5;
		table[XED_CATEGORY_SHIFT] = 9;			table[XED_CATEGORY_SSE] = 3;
		table[XED_CATEGORY_STRINGOP] = 8;		table[XED_CATEGORY_STTNI] = 6;
		table[XED_CATEGORY_SYSCALL] = 7;		table[XED_CATEGORY_SYSRET] = 7;
		table[XED_CATEGORY_SYSTEM] = 6;			table[XED_CATEGORY_UNCOND_BR] = 8;
		table[XED_CATEGORY_VFMA] = 5;			table[XED_CATEGORY_VTX] = 9;
		table[XED_CATEGORY_WIDENOP] = 4;		table[XED_CATEGORY_X87_ALU] = 1;
		table[XED_CATEGORY_XSAVE] = 3;			table[XED_CATEGORY_XSAVEOPT] = 2;

		cacheHitCost = 2;
		cacheMissCost = 3;
	}

	void fillTableFromFile(ifstream &file)
	{
		string line, categoryStr;
		size_t cost;

		while(!file.eof())
		{
			getline(file, line);
			trim(line);
			if(!line.empty() && !starts_with(line, COST_FILE_COMMENT))
			{
				stringstream ss(line);
				ss >> categoryStr;
				ss >> cost;
				if(categoryStr == "CACHE_HIT")
					cacheHitCost = cost;
				else if(categoryStr == "CACHE_MISS")
					cacheMissCost = cost;
				else
					table[str2xed_category_enum_t(categoryStr.c_str())] = cost;
			}
		}
	}
};

#endif /* INSTRCOST_HPP */
