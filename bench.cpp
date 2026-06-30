#include "block.hpp"
#include "search.hpp"
#include "utils.hpp"
#include "board.hpp"
#include "clear.hpp"
#include <cstdio>
#include <iostream>
#include <cassert>
#include <chrono>
#include <cstring>
#include <array>
#include <utility>
using namespace std;
using reachability::operator""_szc;

using BOARD = reachability::board_t<10, 48>;

template <auto policy>
uint64_t perft(BOARD b, const char *block_name, unsigned depth, unsigned height = 0) {
  using namespace reachability;
  return call_with_block<blocks::SRS>(block_from_name(*block_name), [&]<block B>[[gnu::always_inline]](){
    constexpr int relative_height = search::lowest_position<B>;
    return b.call_with_height<reachability::tuple{6, 12, 24, 48}>(height + 3, [&][[gnu::always_inline]](auto nb){
      uint64_t n = 0;
      constexpr coord spawn_pos = coord{4, 20};
      constexpr int necessary_height = spawn_pos[1_szc] + relative_height;
      std::array<decltype(nb), B.shapes> reachable;
      if constexpr (nb.height < necessary_height) {
        reachable = search::binary_bfs<policy, B, spawn_pos, 0, false>(nb);
      } else {
        bool check_consecutive = height > necessary_height;
        if (check_consecutive) [[unlikely]] {
          reachable = search::binary_bfs<policy, B, spawn_pos, 0, true>(nb);
        } else {
          reachable = search::binary_bfs<policy, B, spawn_pos, 0, false>(nb);
        }
      }
      if (depth == 1) {
        for (std::size_t rot = 0; rot < reachable.size(); ++rot)
          n += reachable[rot].popcount();
        return n;
      }
      constexpr int max_width = 4;
      decltype(nb) clear_masks[max_width];
      static_for<max_width>([&][[gnu::always_inline]](auto i){
        clear_masks[int(i)] = clear::has_single_empty_span<i+1>(nb);
      });
      static_for<reachable.size()>([&][[gnu::always_inline]](auto rot){
        constexpr auto mino = B.minos[rot];
        constexpr auto range = blocks::mino_range<mino>();
        constexpr int min_y = range[1], max_y = range[3];
        constexpr auto width_counts = clear::width_counts<mino>();
        constexpr int lines = width_counts.size();
        static_assert(lines == max_y - min_y + 1);
        auto no_clear = reachable[rot];
        static_for<lines>([&][[gnu::always_inline]](auto i){
          static_assert(width_counts[i] <= max_width && width_counts[i] >= 1);
          const auto clear_mask = clear_masks[width_counts[i]-1].template move<coord{0, -(int(i) + min_y)}>();
          no_clear &= ~clear_mask;
        });
        no_clear.for_each_bit([&][[gnu::always_inline]](auto x, auto y) {
          BOARD new_board = b | shapes<BOARD, B.minos>[rot][y % BOARD::lines_per_under].put(x, y, blocks::mino_ranges<B>[rot]);
          unsigned new_height = std::max(height, unsigned(y + max_y + 1));
          n += perft<policy>(new_board, block_name+1, depth-1, new_height);
        });
        (reachable[rot] & ~no_clear).for_each_bit([&][[gnu::always_inline]](auto x, auto y) {
          BOARD new_board = b | shapes<BOARD, B.minos>[rot][y % BOARD::lines_per_under].put(x, y, blocks::mino_ranges<B>[rot]);
          auto [cleared_board, cleared_lines] = new_board.clear_full_lines();
          [[assume(cleared_lines != 0)]];
          unsigned new_height = std::max(height, unsigned(y + max_y + 1)) - cleared_lines;
          n += perft<policy>(cleared_board, block_name+1, depth-1, new_height);
        });
      });
      return n;
    });
  });
}

template <auto policy>
pair<uint64_t, uint64_t> perft_with_time(BOARD b, const char *block, unsigned depth) {
  const auto start = std::chrono::high_resolution_clock::now();
  uint64_t nodes = perft<policy>(b, block, depth);
  const auto end = std::chrono::high_resolution_clock::now();
  const auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return {nodes, dt};
}

template <auto policy>
void test() {
  constexpr std::array test_data = {
    std::pair{"IIIIII"sv, 33325433u},
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
    const uint64_t result = perft<policy>(state, blocks.data(), blocks.size());
    std::cout << "Testing blocks: " << blocks << ", expected: " << expected << ", got: " << result << std::endl;
    assert(result == expected);
  }
}

template <auto policy>
void bench() {
  constexpr std::array data = {
    "IOLJSZT"sv,
    "TIOLJSZ"sv,
    "ZTIOLJS"sv,
    "SZTIOLJ"sv,
    "JSZTIOL"sv,
    "LJSZTIO"sv,
    "OLJSZTI"sv,
  };
  uint64_t nodes_sum = 0, dt_sum = 0;
  for (const auto &blocks : data) {
    const auto [nodes, dt] = perft_with_time<policy>(BOARD{}, blocks.data(), blocks.size());
    std::cout << "Blocks: " << blocks << " Nodes: " << nodes << " Time: " << dt << "ms" << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
    nodes_sum += nodes;
    dt_sum += dt;
  }
  std::cout << "Total Nodes: " << nodes_sum << " Total Time: " << dt_sum << "ms" << " Average NPS: " << (nodes_sum * 1000) / (dt_sum + 1) << std::endl;
}

template <auto policy>
void main_small(char *argv[]) {
  if (strcmp(argv[1], "test") == 0) {
    test<policy>();
    std::cout << "All tests passed!" << std::endl;
    return;
  } else if (strcmp(argv[1], "bench") == 0) {
    bench<policy>();
    return;
  }

  const auto &[nodes, dt] = perft_with_time<policy>(BOARD{}, argv[1], strlen(argv[1]));

  std::cout << "Depth: " << strlen(argv[1])
            << " Nodes: " << nodes
            << " Time: " << dt << "ms"
            << " NPS: " << (nodes * 1000) / static_cast<uint64_t>(dt + 1) << std::endl;
}

int main(int argc, char *argv[]) {
  assert(argc == 2 || argc == 3);
  bool use_shiftdown = false;
  if (argc == 3) {
    if (strcmp(argv[2], "shiftdown") == 0) {
      use_shiftdown = true;
    } else if (strcmp(argv[2], "direct") == 0) {
      use_shiftdown = false;
    } else {
      std::cerr << "Invalid argument: " << argv[2] << std::endl;
      return 1;
    }
  }
  if (use_shiftdown) {
    main_small<reachability::search::shiftdown_reachable>(argv);
  } else {
    main_small<reachability::search::direct_reachable>(argv);
  }
}
