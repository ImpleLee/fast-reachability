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
#include <iostream>
#include <fstream>
#include <cassert>
#include "bench.hpp"
#include <immintrin.h>
using namespace std;

uint64_t perft(BOARD b, const char *block, unsigned depth) {
  const __m512i incremental_bytes = []{
    /* Compiler optimizes this into an initialized array in .rodata. */
    alignas(64) char data[sizeof(__m512i)];
    for (unsigned i = 0; i < sizeof(data); i++) {
        data[i] = i;
    }
    return _mm512_load_si512(data);
  }();
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
        reachability::static_for<4>([&](auto i) {
          uint64_t data_i = reachable[rot].data[i];
          __m512i usable_positions = _mm512_mask_compress_epi8(_mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, 0), data_i, incremental_bytes);
          alignas(64) char data[sizeof(__m512i)];
          _mm512_store_epi64(data, usable_positions);
          int count = std::popcount(data_i);
          for (int k = 0; k < count; ++k) {
            [[assume(data[k] / 10 < 6 && data[k] / 10 >= 0)]];
            BOARD new_board = b.put<mino>(data[k] % 10, data[k] / 10 + i * 6);
            new_board.clear_full_lines();
            n += perft(new_board, block+1, depth-1);
          }
        });
      });
      return n;
    });

    return nodes;
}

constexpr auto error_clear_line_board = merge_str({
  "X         ",
  "XX        ",
  "XXX       ",
  "XXXX      ",
  "XXXXX     ",
  "XXXXXXXXXX",
  "XXXXXX    ",
});

int main(int argc, char *argv[]) {
    BOARD b(error_clear_line_board);
    auto count1 = b.popcount(), removed_count = b.all_bits().populate_highest_bit().popcount();
    b.clear_full_lines();
    auto count2 = b.popcount();
    assert(count2 + removed_count == count1);
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
