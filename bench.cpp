#include "block.hpp"
#include "search.hpp"
#include <string_view>
#include <cstdio>
#include <cmath>
#include "bench.hpp"
using namespace std;

template <bool print=false, reachability::coord start=reachability::coord{4, 20}, unsigned init_rot=0>
double test(const BOARD &b, string_view name, reachability::block_type block) {
  using namespace reachability::search;
  using namespace reachability::blocks;
  printf("BOARD %s\n", name.data());
  printf(" BLOCK %s\n", name_of(block));
  auto binary_time = bench<100000000>([](BOARD b, reachability::block_type block){ return binary_bfs<SRS, start, init_rot>(b, block); }, b, block);
  printf("  binary  : %f cycles\n", binary_time);
  return binary_time;
}
int main() {
  double binary_sum = 0;
  unsigned count = 0;
  using enum reachability::block_type;
  constexpr reachability::block_type blocks[] = {T, Z, S, J, L, O, I};
  for (size_t i = 0; i < board_names.size(); ++i) {
    for (auto block : blocks) {
      auto binary_time = test(boards[i], board_names[i], block);
      binary_sum += binary_time;
      count++;
    }
  }
  printf("AVARAGE binary  : %f cycles\n", binary_sum / count);
}