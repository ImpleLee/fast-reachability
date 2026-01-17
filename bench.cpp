// #include "block.hpp"
// #include "search.hpp"
// #include <string_view>
// #include <cstdio>
// #include <cmath>
// #include "bench.hpp"
// using namespace std;

// template <bool print=false, reachability::coord start=reachability::coord{4, 20}, unsigned init_rot=0>
// double test(const BOARD &b, string_view name, char block) {
//   using namespace reachability::search;
//   using namespace reachability::blocks;
//   printf("BOARD %s\n", name.data());
//   printf(" BLOCK %c\n", block);
//   auto binary_time = bench<100000000>([](BOARD b, char block){ return binary_bfs<SRS, start, init_rot>(b, block); }, b, block);
//   printf("  binary  : %f cycles\n", binary_time);
//   return binary_time;
// }
// int main() {
//   double binary_sum = 0;
//   unsigned count = 0;
//   for (size_t i = 0; i < board_names.size(); ++i) {
//     for (char block : "IJLOSTZ"sv) {
//       auto binary_time = test(boards[i], board_names[i], block);
//       binary_sum += binary_time;
//       count++;
//     }
//   }
//   printf("AVARAGE binary  : %f cycles\n", binary_sum / count);
// }


#include "block.hpp"
#include "search.hpp"
#include "utils.hpp"
#include <string_view>
#include <cstdio>
#include <cmath>
#include <span>
#include "bench.hpp"
using namespace std;
#include <iostream>


uint64_t perft(const BOARD &b, const char *block, unsigned depth) {
    uint64_t nodes = 1;
    auto reachable = reachability::search::binary_bfs<reachability::blocks::SRS, reachability::coord{4,20}, 0uz>(b,*block);
    if (depth == 1) {
        DoNotOptimize(reachable);
        return 1;
    }

    
    for(std::size_t rot = 0; rot < reachable.size(); ++rot) {
      const auto &reachable_board = reachable[rot];
      nodes += reachability::blocks::call_with_block<reachability::blocks::SRS>(*block, [&]<reachability::block B>(){
        uint64_t n = 0;
        reachability::static_for<BOARD::height>([&] (auto y) {
          reachability::static_for<BOARD::width>([&] (auto x) {
            if (reachable_board.template get<x,y>()) {
              BOARD new_board = b;
              reachability::static_for<B.BLOCK_PER_MINO>([&](const std::size_t mino_i) {
                int px = x + B.minos[rot][mino_i][0];
                int py = y + B.minos[rot][mino_i][1];
                new_board.set(px, py);
              });
              new_board.clear_full_lines();
              n += perft(new_board, block+1, depth-1);
            }
          });
        });
        return n;
      });

    }

    return nodes;
}

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
  constexpr unsigned depth = 7;
    const char *queue = "IOLJSZT";
    BOARD state;
    const auto start = std::chrono::high_resolution_clock::now();

    const uint64_t nodes = perft(state, queue, depth);

    const auto end = std::chrono::high_resolution_clock::now();
    const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Depth: " << depth
              << " Nodes: " << nodes
              << " Time: " << dt << "ms"
              << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
}

