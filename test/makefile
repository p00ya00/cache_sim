CC = g++
CFLAGS = -Wall -g -std=c++0x
INCL = -I../src
SRC_DIR = ../src
OUT_DIR = bin
LIBS_DIR = -L/usr/lib
LIBS = -lboost_unit_test_framework

mkdir:
	mkdir -p $(OUT_DIR)

replacement_test: mkdir $(SRC_DIR)/lru.hpp
	$(CC) $(CFLAGS) $(INCL) replacement_policy_test.cpp $(LIBS) -o $(OUT_DIR)/$@
	
full_assoc_test: mkdir $(SRC_DIR)/full_accessor.hpp
	$(CC) $(CFLAGS) $(INCL) full_associativity_test.cpp $(LIBS) -o $(OUT_DIR)/$@

set_assoc_test: mkdir $(SRC_DIR)/set_accessor.hpp
	$(CC) $(CFLAGS) $(INCL) set_associativity_test.cpp $(LIBS) -o $(OUT_DIR)/$@

cache_test: mkdir $(SRC_DIR)/cache.hpp $(SRC_DIR)/main_mem_mock.hpp
	$(CC) $(CFLAGS) $(INCL) cache_test.cpp $(LIBS) -o $(OUT_DIR)/$@

prefetcher_test: mkdir $(SRC_DIR)/stream_prefetcher.hpp $(SRC_DIR)/stride_prefetcher.hpp
	$(CC) $(CFLAGS) $(INCL) $(SRC_DIR)/stream_prefetcher.cpp $(SRC_DIR)/stride_prefetcher.cpp prefetcher_test.cpp $(LIBS) -o $(OUT_DIR)/$@

all: replacement_test set_assoc_test prefetcher_test

run_all: all
	$(OUT_DIR)/replacement_test
	$(OUT_DIR)/full_assoc_test
	$(OUT_DIR)/set_assoc_test
	$(OUT_DIR)/cache_test

clean:
	rm -rf *.o 
