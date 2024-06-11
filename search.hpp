#pragma once
#include "block.hpp"
#include "utils.hpp"
#include <tuple>
#include <queue>
#include <array>
#include <type_traits>
#include <span>

namespace reachability::search {
  using namespace blocks;
  template <std::array mino, typename board_t>
  constexpr board_t usable_positions(board_t data) {
    board_t positions = ~board_t();
    static_for<mino.size()>([&][[gnu::always_inline]](auto i) {
      positions &= (~data).template move<-mino[i]>();
    });
    return positions;
  }
  template <typename board_t>
  constexpr board_t landable_positions(board_t usable) {
    return usable & ~usable.template move<coord{0, 1}>();
  }
  template <typename board_t>
  constexpr board_t consecutive_lines(board_t usable) {
    const auto indicator01 = usable & ~usable.template move<coord{1, 0}>();
    return indicator01.has_single_bit();
  }
  template <std::array kick, typename board_t>
  constexpr std::array<board_t, kick.size()>
      kick_positions(board_t start, board_t end) {
    constexpr std::size_t N = kick.size();
    std::array<board_t, N> positions;
    static_for<N>([&][[gnu::always_inline]](auto i) {
      positions[i] = start & end.template move<-kick[i]>();
    });
    board_t temp = positions[0];
    static_for<N-1>([&][[gnu::always_inline]](auto i) {
      positions[i + 1] &= ~temp;
      temp |= positions[i + 1];
    });
    return positions;
  }
  template <block block, coord start, unsigned init_rot, typename board_t>
  constexpr std::array<board_t, block.SHAPES> binary_bfs(board_t data) {
    constexpr int orientations = block.ORIENTATIONS;
    constexpr int rotations = block.ROTATIONS;
    constexpr int kicks = block.KICK_PER_ROTATION;
    constexpr int shapes = block.SHAPES;
    board_t usable[shapes];
    std::array<board_t, kicks> kicks2[orientations][rotations];
    static_for<shapes>([&][[gnu::always_inline]](auto i) {
      usable[i] = usable_positions<block.minos[i]>(data);
    });
    static_for<orientations>([&][[gnu::always_inline]](auto i) {
      constexpr auto index = block.mino_index[i];
      static_for<rotations>([&][[gnu::always_inline]](auto j) {
        constexpr auto target = block.rotation_target(i, j);
        constexpr auto index2 = block.mino_index[target];
        kicks2[i][j] = kick_positions<block.kicks[i][j]>(usable[index], usable[index2]);
      });
    });
    constexpr std::array<coord, 3> MOVES = {{{-1, 0}, {1, 0}, {0, -1}}};
    constexpr coord start2 = start + block.mino_offset[init_rot];
    constexpr auto init_rot2 = block.mino_index[init_rot];
    if (!usable[init_rot2].template get<start2[0], start2[1]>()) [[unlikely]] {
      return {};
    }
    bool need_visit[orientations] = { };
    need_visit[init_rot] = true;
    std::array<board_t, orientations> cache;
    const auto consecutive = consecutive_lines(usable[init_rot2]).populate_highest_bit();
    if (consecutive.template get<start2[0], start2[1]>()) [[likely]] {
      const auto consecutive_usable = consecutive & usable[init_rot2];
      const auto indicator11 = consecutive_usable & consecutive_usable.template move<coord{0, -1}>();
      auto maybe_usable = indicator11.any_bit().populate_highest_bit();
      constexpr int removed_lines = board_t::height - start2[1];
      if constexpr (removed_lines > 0) {
        maybe_usable |= ~(~board_t()).template move<coord{0, -removed_lines}>();
      }
      auto good_lines = maybe_usable.remove_ones_after_zero();
      if constexpr (removed_lines > 1) {
        good_lines &= (~board_t()).template move<coord{0, -(removed_lines - 1)}>();
      }
      cache[init_rot] = good_lines & usable[init_rot2];
    } else {
      cache[init_rot].template set<start2[0], start2[1]>();
    }
    for (bool updated = true; updated;) [[unlikely]] {
      updated = false;
      static_for<orientations>([&][[gnu::always_inline]](auto i){
        if (!need_visit[i]) {
          return;
        }
        constexpr auto index = block.mino_index[i];
        need_visit[i] = false;
        while (true) {
          board_t result;
          static_for<MOVES.size()>([&][[gnu::always_inline]](auto j) {
            result |= cache[i].template move<MOVES[j]>();
          });
          result &= usable[index];
          if ((result & ~cache[i]).any()) [[likely]] {
            cache[i] |= result;
          } else {
            break;
          }
        }
        static_for<rotations>([&][[gnu::always_inline]](auto j){
          constexpr int target = block.rotation_target(i, j);
          board_t to;
          static_for<kicks>([&][[gnu::always_inline]](auto k){
            to |= (cache[i] & kicks2[i][j][k]).template move<block.kicks[i][j][k], false>();
          });
          board_t old_cache = cache[target];
          cache[target] |= to;
          if ((to & ~old_cache).any()) {
            need_visit[target] = true;
            if (target < i)
              updated = true;
          }
        });
      });
    }
    std::array<board_t, shapes> ret;
    static_for<orientations>([&][[gnu::always_inline]](auto i){
      constexpr auto index = block.mino_index[i];
      ret[index] |= cache[i];
    });
    static_for<shapes>([&][[gnu::always_inline]](auto i){
      ret[i] &= landable_positions(usable[i]);
    });
    return ret;
  }
  template <typename RS, coord start, unsigned init_rot=0, typename board_t>
  [[gnu::noinline]]
  constexpr static_vector<board_t, 4> binary_bfs(board_t data, char b) {
    return call_with_block<RS>(b, [=]<block B>() {
      auto ret = binary_bfs<B, start, init_rot>(data);
      return static_vector<board_t, 4>{std::span{ret}};
    });
  }
  template <typename board_t>
  auto ordinary_bfs_without_binary(board_t data, const auto &block, const coord &start, unsigned init_rot) {
    constexpr auto orientations = std::remove_cvref_t<decltype(block)>::ORIENTATIONS;
    constexpr auto shapes = std::remove_cvref_t<decltype(block)>::SHAPES;
    constexpr auto W = board_t::width;
    constexpr auto H = board_t::height;
    bool my_data[H][W];
    static_for<H>([&][[gnu::always_inline]](auto y) {
      static_for<W>([&][[gnu::always_inline]](auto x) {
        my_data[y][x] = data.template get<x, y>();
      });
    });
    auto in_range = [](int x, int y) {
      return 0 <= x && unsigned(x) < W && 0 <= y && unsigned(y) < H;
    };
    auto usable = [&](int i, int x, int y) {
      const auto index = block.mino_index[i];
      for (const auto &[dx, dy] : block.minos[index]) {
        if (!in_range(x + dx, y + dy)) return false;
        if (my_data[y + dy][x + dx]) return false;
      }
      return true;
    };
    constexpr const coord MOVES[] = {{0, -1}, {-1, 0}, {1, 0}};
    bool visited[orientations][H][W] = {};
    bool ret[orientations][H][W] = {};
    std::queue<std::tuple<int, int, int>> q;
    auto visit = [&] [[gnu::always_inline]] (int i, int x, int y) {
      if (!in_range(x, y) || !usable(i, x, y))
        return false;
      if (!visited[i][y][x]) {
        visited[i][y][x] = true;
        q.emplace(x, y, i);
        if (!usable(i, x, y - 1))
          ret[i][y][x] = true;
      }
      return true;
    };
    const auto start2 = start + block.mino_offset[init_rot];
    visit(init_rot, start2[0], start2[1]);
    while (!q.empty()) {
      const auto [x, y, i] = q.front();
      q.pop();
      for (auto &[dx, dy] : MOVES) {
          visit(i, x + dx, y + dy);
      }
      for (int j = 0; j < block.ROTATIONS; ++j) {
        const auto target = block.rotation_target(i, j);
        for (auto &[dx, dy] : block.kicks[i][j]) {
          if (visit(target, x + dx, y + dy)) {
            break;
          }
        }
      }
    }
    std::array<board_t, shapes> true_ret;
    static_for<orientations>([&][[gnu::always_inline]](auto i) {
      auto index = block.mino_index[i];
      static_for<H>([&][[gnu::always_inline]](auto y) {
        static_for<W>([&][[gnu::always_inline]](auto x) {
          if (ret[i][y][x]) {
            true_ret[index].template set<x, y>();
          }
        });
      });
    });
    return true_ret;
  }
  template <typename RS, typename board_t>
  [[gnu::noinline]]
  constexpr static_vector<board_t, 4> ordinary_bfs_without_binary(board_t data, char b, const coord &start, unsigned init_rot=0) {
    return call_with_block<RS>(b, [=]<block B>() {
      auto ret = ordinary_bfs_without_binary(data, B, start, init_rot);
      return static_vector<board_t, 4>{std::span{ret}};
    });
  }
}