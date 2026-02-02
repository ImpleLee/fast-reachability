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

using BOARD = reachability::board_t<10, 24>;

uint64_t perft(BOARD b, const char *block, unsigned depth) {
  return reachability::call_with_block<reachability::blocks::SRS>(reachability::block_from_name(*block), [&]<reachability::block B>(){
    auto reachable = reachability::search::binary_bfs<B, reachability::coord{4,20}, 0>(b);
    uint64_t n = 0;
    if (depth == 1) {
      for (std::size_t rot = 0; rot < reachable.size(); ++rot)
        n += reachable[rot].popcount();
      return n;
    }
    reachability::static_for<B.shapes>([&](auto rot) {
      constexpr auto mino = std::get<rot>(B.minos);
      reachable[rot].for_each_bit([&](int x, int y) {
        BOARD new_board = b | BOARD::put<mino>(x, y);
        n += perft(new_board.clear_full_lines().first, block+1, depth-1);
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
