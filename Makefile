MAKEFLAGS += rR
CC = g++
LIB_FLAGS = -std=c++2b
OPT_FLAGS = -O3 -march=native
LINK_FLAGS = -flto=auto -g -gz
EXTRA_FLAGS = -pipe -MMD
DEBUG_FLAGS = -Wall -Wextra -Werror=shift-count-negative -Werror=shift-count-overflow
CXXFLAGS = $(LIB_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(EXTRA_FLAGS)
TARGETS := $(patsubst %.cpp, build/%, $(wildcard *.cpp))

run: build/bench
	taskset --cpu-list 0 $<

build/%: %.cpp build
	$(CC) $< -o $@ $(CXXFLAGS) $(LINK_FLAGS)

build/%.s: %.cpp build
	$(CC) $< -o $@ $(CXXFLAGS) -S

build:
	mkdir -p build

.PHONY: clean all run
all: $(TARGETS)
clean:
	rm -rf build

-include $(TARGETS:=.d)