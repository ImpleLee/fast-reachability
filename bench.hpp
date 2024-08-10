#pragma once
#include <array>
#include <initializer_list>
#include <string_view>
#include <chrono>
#include "board.hpp"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

constexpr int WIDTH = 10, HEIGHT = 24;
using BOARD = reachability::board_t<WIDTH, HEIGHT>;
constexpr auto merge_str(std::initializer_list<std::string_view> &&b_str) {
  std::array<char, WIDTH*HEIGHT> res = {};
  unsigned pos = 0;
  if (b_str.size() < HEIGHT) {
    for (pos = 0; pos < WIDTH * (HEIGHT - b_str.size()); ++pos) {
      res[pos] = ' ';
    }
  }
  for (auto &s : b_str) {
    for (unsigned i = 0; i < WIDTH; ++i) {
      res[pos + i] = s[WIDTH - i - 1];
    }
    pos += WIDTH;
  }
  return BOARD::convert_to_array(std::string_view{res.data(), res.size()});
}

inline constexpr std::array boards = {
  merge_str({
    "X         ",
    "X         ",
    "XX      XX",
    "XXX    XXX",
    "XXXX   XXX",
    "XXXX  XXXX",
    "XXX   XXXX",
    "XXXX XXXXX"
  }),
  merge_str({
    "        XX",
    "XXXX  XXXX",
    "XXX   XXXX",
    "XXX XXXXXX",
    "XXX  XXXXX",
    "XX   XXXXX",
    "XXX XXXXXX",
    "XXX XXXXXX",
    "XXXX XXXXX"
  }),
  merge_str({
    "XXXXXXXX  ",
    "XXXXXXXX  ",
    "X         ",
    "X         ",
    "X  XXXXXXX",
    "X  XXXXXXX",
    "X        X",
    "X        X",
    "XXXXXXXX X",
    "XXXXXXXX X",
    "         X",
    "         X"
  }),
  merge_str({
    "   XX     ",
    "X XX    XX",
    "XXXX XXXXX",
    "XXXX   XXX",
    "XXX     XX",
    "XXXXXX XXX",
    "XXXX   XXX",
    "XXXXX XXXX",
    "XXXX XXXXX",
    "XXXX XXXXX"
  })
};
inline constexpr std::array board_names = {
  "LEMONTEA TSPIN", "LEMONTEA DT", "LEMONTEA TERRIBLE", "4T"
};

// from https://github.com/facebook/folly/blob/7a3f5e4e81bc83a07036e2d1d99d6a5bf5932a48/folly/lang/Hint-inl.h#L107
// Apache License 2.0
template <class Tp>
inline void DoNotOptimize(Tp& value) {
  asm volatile("" : : "m"(value) : "memory");
}
template <class Tp>
inline void DoNotOptimize(const Tp &value) {
  asm volatile("" : : "m"(value) : "memory");
}

template <int count = 50000>
auto bench(auto f, auto &&...args) {
  auto start = __rdtsc();
  for (int _ = 0; _ < count; ++_) {
    (void)(DoNotOptimize(args), ...);
    DoNotOptimize(f(args...));
  }
  auto end = __rdtsc();
  return double(end - start) / count;
}