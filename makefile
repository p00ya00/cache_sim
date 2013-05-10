## To build the tool, execute the make command:
##
##      make
## or
##      make PIN_HOME=<top-level directory where Pin was installed>
##
## After building your tool, you would invoke Pin like this:
## 
##      $PIN_HOME/pin -t MyPinTool -- /bin/ls

# 1. Change PIN_HOME to point to the top-level directory where
#    Pin was installed. This can also be set on the command line,
#    or as an environment variable.
#
PIN_HOME ?= $PIN_HOME


# set up and include *.config files
PIN_KIT=$(PIN_HOME)
KIT=1
TESTAPP=$(OBJDIR)cp-pin.exe

TARGET_COMPILER?=gnu
ifdef OS
    ifeq (${OS},Windows_NT)
        TARGET_COMPILER=ms
    endif
endif

ifeq ($(TARGET_COMPILER),gnu)
    include $(PIN_HOME)/source/tools/makefile.gnu.config
    CXXFLAGS ?= -Wall -Werror -Wno-unknown-pragmas $(DBG) $(OPT)
    PIN=$(PIN_HOME)/pin
endif

ifeq ($(TARGET_COMPILER),ms)
    include $(PIN_HOME)/source/tools/makefile.ms.config
    DBG?=
    PIN=$(PIN_HOME)/pin.bat
endif

# Tools - you may wish to add your tool name to TOOL_ROOTS
TOOL_ROOTS = memtrace
TOOLS = $(TOOL_ROOTS:%=$(OBJDIR)%$(PINTOOL_SUFFIX))

SRC_DIR = src/
#OBJS = $(OBJDIR)memtrace.o $(OBJDIR)cache.o
OBJS = $(OBJDIR)memtrace.o $(OBJDIR)stream_prefetcher.o $(OBJDIR)stride_prefetcher.o

all: tools
tools: $(OBJDIR) $(TOOLS) $(OBJDIR)cp-pin.exe
test: $(OBJDIR) $(TOOL_ROOTS:%=%.test)

MyPinTool.test: $(OBJDIR)cp-pin.exe
	$(MAKE) -k PIN_HOME=$(PIN_HOME)

$(OBJDIR)cp-pin.exe:
	$(CXX) $(PIN_HOME)/source/tools/Tests/cp-pin.cpp $(APP_CXXFLAGS) -o $(OBJDIR)cp-pin.exe -lrt

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)%.o : $(SRC_DIR)%.cpp
	$(CXX) -c -g -std=c++0x $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ $< -lrt

$(TOOLS): $(PIN_LIBNAMES)

$(TOOLS): %$(PINTOOL_SUFFIX) : $(OBJS)
	${PIN_LD} $(PIN_LDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $^ ${PIN_LPATHS} $(PIN_LIBS) -lrt $(DBG)

clean:
	-rm -rf $(OBJDIR) *.out *.tested *.failed makefile.copy
