CC = g++
LIB_FLAGS = -std=c++2b
OPT_FLAGS = -O3 -march=native -flto=auto
DEBUG_FLAGS = -g -Wall -Wextra -Werror=shift-count-negative -Werror=shift-count-overflow
CXXFLAGS = $(LIB_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS)
TARGETS := $(patsubst %.cpp, build/%, $(wildcard *.cpp))

run: all
	./build/main

build/%: %.cpp build
	$(CC) $(CXXFLAGS) $< -o $@

build:
	mkdir -p build

.PHONY: clean all build run time bench
all: $(TARGETS)
clean:
	rm -rf build

bench:
	touch $$(git rev-parse HEAD)-bench
	git add $$(git rev-parse HEAD)-bench
	git stash
	git cherry-pick bench --no-commit
	git stash apply
	make all
	samply record --save-only -o build/profile.json build/main
	git reset --hard
	git stash pop
	git restore --staged $$(git rev-parse HEAD)-bench
	rm $$(git rev-parse HEAD)-bench