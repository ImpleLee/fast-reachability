#include "block.hpp"
#include "search.hpp"
#include "utils.hpp"
#include "board.hpp"
#include <cstdio>
#include <iostream>
#include <cassert>
#include <chrono>
#include <cstring>
#include <array>
#include <utility>
using namespace std;

using BOARD = reachability::board_t<10, 48>;

uint64_t perft(BOARD b, const char *block, unsigned depth, unsigned height = 0) {
  return reachability::call_with_block<reachability::blocks::SRS>(reachability::block_from_name(*block), [&]<reachability::block B>(){
    uint64_t n = 0;
    b.call_with_height<reachability::tuple{6, 12, 24, 48}>(height + 4, [&](auto nb){
      constexpr reachability::coord spawn_pos = reachability::coord{4, std::min(22, nb.height) - 2};
      auto reachable = reachability::search::binary_bfs<B, spawn_pos, 0>(nb);
      if (depth == 1) {
        for (std::size_t rot = 0; rot < reachable.size(); ++rot)
          n += reachable[rot].popcount();
        return;
      }
      reachability::static_for<B.shapes>([&](auto rot) {
        constexpr auto mino = B.minos[rot];
        constexpr auto range = reachability::blocks::mino_range<mino>();
        constexpr auto max_y = range[3];
        reachable[rot].for_each_bit([&](int x, int y) {
          BOARD new_board = b | BOARD::put<mino>(x, y);
          unsigned new_height = std::max(height, unsigned(y + max_y + 1));
          n += perft(new_board.clear_full_lines().first, block+1, depth-1, new_height);
        });
      });
    });
    return n;
  });
}

void test() {
  constexpr std::array test_data = {
    std::pair{"IOLJSZT"sv, 2647076135u},
    std::pair{"TIOLJSZ"sv, 2785677550u},
    std::pair{"ZTIOLJS"sv, 2741273038u},
    std::pair{"SZTIOLJ"sv, 2740055656u},
    std::pair{"JSZTIOL"sv, 2801460686u},
    std::pair{"LJSZTIO"sv, 2852978763u},
    std::pair{"OLJSZTI"sv, 2689379684u},
  };
  for (const auto &[blocks, expected] : test_data) {
    BOARD state;
    const uint64_t result = perft(state, blocks.data(), blocks.size());
    std::cout << "Testing blocks: " << blocks << ", expected: " << expected << ", got: " << result << std::endl;
    assert(result == expected);
  }
}

int main(int argc, char *argv[]) {
    assert(argc == 2);
    if (strcmp(argv[1], "test") == 0) {
      test();
      std::cout << "All tests passed!" << std::endl;
      return 0;
    }

    BOARD state;
    const auto start = std::chrono::high_resolution_clock::now();

    const uint64_t nodes = perft(state, argv[1], strlen(argv[1]));

    const auto end = std::chrono::high_resolution_clock::now();
    const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Depth: " << strlen(argv[1])
              << " Nodes: " << nodes
              << " Time: " << dt << "ms"
              << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
}
