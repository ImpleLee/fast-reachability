CC = g++
CXXFLAGS = -std=c++17 -g -Og -Wall -Wextra
TARGETS := $(patsubst %.cpp, build/%, $(wildcard *.cpp))

all: $(TARGETS)

build/%: %.cpp build
	$(CC) $(CXXFLAGS) $< -o $@

build:
	mkdir -p build

.PHONY: clean all build
clean:
	rm -rf build