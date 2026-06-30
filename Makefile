MAKEFLAGS += rR
CC = clang++
LIB_FLAGS = -std=c++2b
OPT_FLAGS = -O3 -march=native
LINK_FLAGS = -flto=auto
EXTRA_FLAGS = # this line is only used to allow shell completion
override EXTRA_FLAGS += -pipe -MMD -MP
DEBUG_FLAGS = -Wall -Wextra -Werror=shift-count-negative -Werror=shift-count-overflow
CXXFLAGS = $(LIB_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(EXTRA_FLAGS)
TARGETS := $(patsubst %.cpp, build/%, $(wildcard *.cpp))

run: clean build/bench
	taskset --cpu-list 0 ./build/bench IOLJSZT

build/%: %.cpp build
	$(CC) $< -o $@ $(LINK_FLAGS) $(CXXFLAGS)

build/%.s: %.cpp build
	$(CC) $< -o $@ -S $(CXXFLAGS)

build/%.o.json: %.cpp build
	$(CC) $< -o /dev/null  -MJ $@ -c $(CXXFLAGS)

build:
	mkdir -p build

compile_commands.json: $(TARGETS:=.o.json)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $^ > $@

.PHONY: clean all run
all: $(TARGETS)
clean:
	rm -rf build

-include $(TARGETS:=.d)
