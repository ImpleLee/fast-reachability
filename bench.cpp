#include "block.hpp"
#include "search.hpp"
#include <string_view>
#include <cstdio>
#include <cmath>
#include "bench.hpp"
using namespace std;

template <bool print=false, reachability::coord start=reachability::coord{4, 20}, unsigned init_rot=0>
double test(const BOARD &b, string_view name, char block) {
  using namespace reachability::search;
  using namespace reachability::blocks;
  printf("BOARD %s\n", name.data());
  printf(" BLOCK %c\n", block);
  auto binary_time = bench<100000000>([](BOARD b, char block){ return binary_bfs<SRS, start, init_rot>(b, block); }, b, block);
  printf("  binary  : %f cycles\n", binary_time);
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
  printf("AVARAGE binary  : %f cycles\n", binary_sum / count);
}