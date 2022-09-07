export AFL_QUIET = 1
CC = afl-clang
CXX = afl-clang++
LD = afl-clang++

ifeq ($(ASAN),1)
export AFL_USE_ASAN = 1
endif

# Prevent NDEBUG from being defined since we will always want assertions to run
# when the code is being fuzzed
SFLAGS := $(filter-out -DNDEBUG,$(SFLAGS))
