CC = g++
LIB_FLAGS = -std=c++2b
OPT_FLAGS = -O3 -march=native -flto=auto
EXTRA_FLAGS =
# use a user-provided flag to disable the gvn-memdep pass
CLANG_FLAGS = -mllvm --enable-gvn-memdep=false
ifeq ($(GVN_MEMDEP), 0)
	ifeq ($(findstring clang, $(CC)), clang)
		OPT_FLAGS += $(CLANG_FLAGS)
	endif
endif
DEBUG_FLAGS = -g -Wall -Wextra -Werror=shift-count-negative -Werror=shift-count-overflow
CXXFLAGS = $(LIB_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(EXTRA_FLAGS)
TARGETS := $(patsubst %.cpp, build/%, $(wildcard *.cpp))

run: all
	./build/main

build/%: %.cpp build
	$(CC) $< -o $@ $(CXXFLAGS)

build:
	mkdir -p build

.PHONY: clean all build run time bench
all: $(TARGETS)
clean:
	rm -rf build

bench: build/bench
	samply record --save-only -o build/profile.json build/bench