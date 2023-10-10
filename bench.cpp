#include "block.hpp"
#include "board.hpp"
#include "search.hpp"
#include <string_view>
#include <cstdio>
#include <cmath>
#include "bench.hpp"
using namespace std;

using BOARD = reachability::board_t<WIDTH, HEIGHT>;
template <bool print=false, reachability::coord start=reachability::coord{4, 20}, unsigned init_rot=0>
double test(const BOARD &b, string_view name, char block) {
  using namespace reachability::search;
  using namespace reachability::blocks;
  printf("BOARD %s\n", name.data());
  printf(" BLOCK %c\n", block);
  auto binary_time = bench([&](){binary_bfs<SRS, start, init_rot>(b, block);}, 1000000);
  printf("  binary  : %fns\n", binary_time);
  return binary_time;
}
int main() {
  double binary_sum = 0;
  unsigned count = 0;
  for (size_t i = 0; i < board_names.size(); ++i) {
    for (char block : "IJLOSTZ"sv) {
      auto binary_time = test(boards[i], board_names[i], block);
      binary_sum += binary_time;
      count++;
    }
  }
  auto hz = 1e9 / (binary_sum / count);
  // automatically use k, M, G, T, P, E, Z, Y
  size_t i = log10(hz) / 3;
  char suffix[] = " kMGTPEZY";
  if (i >= sizeof(suffix)) {
    i = sizeof(suffix) - 1;
  }
  printf("AVERAGE binary  : %fns (%f%cHz)\n", binary_sum / count, hz / pow(1000, i), suffix[i]);
}