CC = g++
CXXFLAGS = -std=c++17 -g -O3 -Wall -Wextra -march=native
TARGETS := $(patsubst %.cpp, build/%, $(wildcard *.cpp))

run: all
	./build/main

build/%: %.cpp build
	$(CC) $(CXXFLAGS) $< -o $@

build:
	mkdir -p build

.PHONY: clean all build run time
all: $(TARGETS)
time: all
	time ./build/main >/dev/null
clean:
	rm -rf build