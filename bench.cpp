#include "block.hpp"
#include "search.hpp"
#include "utils.hpp"
#include <string_view>
#include <cstdio>
#include <cmath>
#include <span>
#include <iostream>
#include <fstream>
#include <cassert>
#include "bench.hpp"
using namespace std;

uint64_t perft(BOARD b, const char *block, unsigned depth) {
    uint64_t nodes = 0;
    auto reachable = reachability::search::binary_bfs<reachability::blocks::SRS, reachability::coord{4,20}, 0uz>(b,*block);
    if (depth == 1) {
        for(std::size_t rot = 0; rot < reachable.size(); ++rot)
            nodes += reachable[rot].popcount();
        return nodes;
    }
    
    nodes += reachability::blocks::call_with_block<reachability::blocks::SRS>(*block, [&]<reachability::block B>(){
      uint64_t n = 0;
      reachability::static_for<B.SHAPES>([&](auto rot) {
        constexpr auto mino = B.minos[rot];
        reachability::static_for<BOARD::height>([&](auto y) {
          reachability::static_for<BOARD::width>([&](auto x) {
            if (reachable[rot].template get<x, y>()) {
              BOARD new_board = b.put<mino>(x, y);
              new_board.clear_full_lines();
              n += perft(new_board, block+1, depth-1);
            }
          });
        });
      });
      return n;
    });

    return nodes;
}

int main(int argc, char *argv[]) {
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
