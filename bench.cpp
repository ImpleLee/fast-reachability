#include "block.hpp"
#include "search.hpp"
#include "utils.hpp"
#include "board.hpp"
#include <cstdio>
#include <iostream>
#include <cassert>
#include <chrono>
#include <cstring>
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
    reachability::static_for<B.SHAPES>([&](auto rot) {
      constexpr auto mino = B.minos[rot];
      reachable[rot].for_each_bit([&](int x, int y) {
        BOARD new_board = b | BOARD::put<mino>(x, y);
        n += perft(new_board.clear_full_lines().first, block+1, depth-1);
      });
    });
    return n;
  });
}

int main(int argc, char *argv[]) {
    assert(argc == 2);
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
