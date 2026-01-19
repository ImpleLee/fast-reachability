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
#include <immintrin.h>
using namespace std;

uint64_t perft(BOARD b, const char *block, unsigned depth) {
  const __m256i incremental_bytes = []{
    /* Compiler optimizes this into an initialized array in .rodata. */
    alignas(64) char data[sizeof(__m256i)];
    for (unsigned i = 0; i < sizeof(data); i++) {
        data[i] = i;
    }
    return _mm256_load_si256((__m256i*)data);
  }();

  return reachability::blocks::call_with_block<reachability::blocks::SRS>(*block, [&]<reachability::block B>(){
    auto reachable = reachability::search::binary_bfs<B, reachability::coord{4,20}, 0>(b);
    uint64_t n = 0;
    if (depth == 1) {
      for (std::size_t rot = 0; rot < reachable.size(); ++rot)
        n += reachable[rot].popcount();
      return n;
    }
    reachability::static_for<B.SHAPES>([&](auto rot) {
      constexpr auto mino = B.minos[rot];
      reachability::static_for<BOARD::num_of_under>([&](auto i) {
        uint64_t data_i = reachable[rot].data[i];
        reachability::static_for<BOARD::under_bits / 32>([&](auto j) {
          uint32_t this_data = data_i >> (j * 30);
          this_data &= (~uint32_t(0)) >> 2;
          __m256i usable_positions = _mm256_maskz_compress_epi8(this_data, incremental_bytes);
          uint8_t *data = reinterpret_cast<uint8_t*>(&usable_positions);
          int count = std::popcount(this_data);
          for (int k = 0; k < count; ++k) {
            [[assume(data[k] / 10 < 3 && data[k] / 10 >= 0)]];
            BOARD new_board = b | BOARD::put<mino>(data[k] % 10, data[k] / 10 + i * 6 + j * 3);
            n += perft(new_board.clear_full_lines().first, block+1, depth-1);
          }
        });
      });
    });
    return n;
  });
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
