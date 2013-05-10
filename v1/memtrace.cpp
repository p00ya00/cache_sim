#define __STDC_FORMAT_MACROS
#include <stdlib.h>
#include <iomanip>
#include <inttypes.h>
#include <stdio.h>
#include <cstring>
#include "pin.H"
#include "cache.hpp"
#include "miss_handler.hpp"
#include "main_mem_mock.hpp"
#include "replacement_policy.hpp"
#include "associativity.hpp"
#include "instr_cost.hpp"
#include "prefetcher.hpp"

/* TODO:
 * - whether read is from code or prefetcher?
 * - add prefetcher
 * - use inst cost correctly and check for instrumentation code
 * - test, test, test!!!
 */

InstrCostTable costTable("instr_cost");

MainMemMock mm;
MissHandler mh;

//typedef CacheConfiguration<LeastRecentlyUsed, FullAssociative, WRITE_THROUGH, MainMemMock> L1Type;
//L1Type configL1(32 * 1024);
//Cache<L1Type> l1cache(configL1, mm);

typedef CacheConfiguration<LeastRecentlyUsed, SetAssociative, WRITE_BACK, MainMemMock> L2Type;
L2Type configL2(128 * 4 * CACHE_LINE_SIZE, 4);
Cache<L2Type> l2cache(configL2, mm, NULL, 2);

typedef CacheConfiguration<LeastRecentlyUsed, SetAssociative, WRITE_BACK, Cache<L2Type> > L1Type;
L1Type configL1(32 * 1024, 8 );
Cache<L1Type> l1cache(configL1, l2cache, ascendingAccess);

size_t cycles = 0;
PIN_MUTEX mutex;

void RecordMemRef(void * ip, void * addr, UINT32 size, OS_THREAD_ID tid, 
                  void *img_name, void *rtn_name, bool write, UINT64 timestamp)
{
	CacheOperationResult res;
	PIN_MutexLock(&mutex);
	res = l1cache.load((Address)addr);
	cycles += costTable.lookup(res);
	if(res == MISS || res == PREFETCH_MISS)
	{
		mh.sendComp(cycles);
		cycles = 0;
		mh.sendRW((Address)addr, size, tid, (char *)rtn_name, timestamp, write,
				(res == PREFETCH_MISS) ? true : false);
	}
    PIN_MutexUnlock(&mutex);
}


// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    RTN rtn;
    SEC sec;
    IMG img;
    const char *img_name = "N/A";
    const char *rtn_name = "N/A";
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    OS_THREAD_ID tid = PIN_GetTid();
    
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // The IA-64 architecture has explicitly predicated instructions. 
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.

    rtn = INS_Rtn(ins);
    if(RTN_Valid(rtn))
    {
        rtn_name = RTN_Name(rtn).c_str();
        sec = RTN_Sec(rtn);
        if(SEC_Valid(sec))
        {
            img = SEC_Img(sec);
            if(IMG_Valid(img))
                img_name = IMG_Name(img).c_str();
        }
    }
    
    cycles += costTable.lookup(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRef,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, IARG_MEMORYREAD_SIZE,
                IARG_UINT32, tid, IARG_PTR, img_name, IARG_PTR, rtn_name,
                IARG_BOOL, false, IARG_TSC, IARG_END);
        }

        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRef,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, IARG_MEMORYWRITE_SIZE,
                IARG_UINT32, tid, IARG_PTR, img_name, IARG_PTR, rtn_name,
                IARG_BOOL, true, IARG_TSC, IARG_END);
        }
    }
}

VOID Fini(INT32 code, void *v)
{
    cout << dec << endl;
	mm.printStats();
	cout << endl;
	l1cache.printStats();
	cout << endl;
	l2cache.printStats();
	cout << endl;
}

   
INT32 Usage()
{
    PIN_ERROR("This Pintool prints a trace of memory addresses\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

int main(int argc, char *argv[])
{
	if (PIN_Init(argc, argv)) 
        return Usage();
	INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_InitSymbols();
    // Never returns
    PIN_StartProgram();
    
    return 0;
}
