CC = g++
CXXFLAGS = -std=c++2b -g -flto=auto -O3 -Wall -Wextra -Werror -march=native
TARGETS := $(patsubst %.cpp, build/%, $(wildcard *.cpp))

run: all
	./build/main

build/%: %.cpp build
	$(CC) $(CXXFLAGS) $< -o $@

build:
	mkdir -p build

.PHONY: clean all build run time
all: $(TARGETS)
clean:
	rm -rf build