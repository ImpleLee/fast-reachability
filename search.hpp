#pragma once
#include "block.hpp"
#include "board.hpp"
#include "utils.hpp"
#include <tuple>
#include <queue>
#include <array>
#include <type_traits>

namespace reachability::search {
  using namespace blocks;
  template <unsigned W, unsigned H, int dx>
  static constexpr board_t<W, H> MASK = []() consteval {
    board_t<W, H> mask;
    if constexpr (dx < 0) {
      static_for<-dx>([&](auto i) {
        static_for<H>([&](auto j) {
          mask.template set<i, j>();
        });
      });
    } else if constexpr (dx > 0) {
      static_for<dx>([&](auto i) {
        static_for<H>([&](auto j) {
          mask.template set<W - 1 - i, j>();
        });
      });
    }
    return ~mask;
  }();
  template <coord d, bool reverse = false, bool check = true, unsigned W, unsigned H>
  constexpr board_t<W, H> move_to_center(board_t<W, H> board) {
    constexpr auto dx = reverse ? -d[0] : d[0];
    constexpr auto dy = reverse ? -d[1] : d[1];
    constexpr int move = dy * W + dx;
    if constexpr (dy == 0) {
      if constexpr (move < 0) {
        board.template left_shift<-move>();
      } else if constexpr (move > 0) {
        board.template right_shift<move>();
      }
    } else {
      if constexpr (move < 0) {
        board.template left_shift_carry<-move>();
      } else if constexpr (move > 0) {
        board.template right_shift_carry<move>();
      }
    }
    if constexpr (check && dx != 0) {
      board &= MASK<W, H, dx>;
    }
    return board;
  }
  template <std::array mino, unsigned W, unsigned H>
  constexpr board_t<W, H> usable_positions(const board_t<W, H> &data) {
    board_t positions = ~board_t<W, H>();
    board_t temp = ~data;
    static_for<std::tuple_size_v<decltype(mino)>>([&](auto i) {
      positions &= move_to_center<mino[i]>(temp);
    });
    return positions;
  }
  template <unsigned W, unsigned H>
  constexpr board_t<W, H> landable_positions(const board_t<W, H> &usable) {
    auto temp = usable;
    temp.template left_shift_carry<W>();
    return usable & ~temp;
  }
  template <std::array kick, unsigned W, unsigned H>
  constexpr std::array<board_t<W, H>, std::tuple_size_v<decltype(kick)>>
      kick_positions(const board_t<W, H> &start, const board_t<W, H> &end) {
    constexpr std::size_t N = std::tuple_size_v<decltype(kick)>;
    std::array<board_t<W, H>, N> positions;
    static_for<N>([&](auto i) {
      positions[i] = move_to_center<kick[i]>(end);
      positions[i] &= start;
    });
    board_t temp = positions[0];
    static_for<N-1>([&](auto i) {
      positions[i + 1] &= ~temp;
      temp |= positions[i + 1];
    });
    return positions;
  }
  template <block block, coord start, unsigned init_rot, unsigned W, unsigned H>
  constexpr std::array<board_t<W, H>, block.ORIENTATIONS> binary_bfs(const board_t<W, H> &data) {
    constexpr int orientations = block.ORIENTATIONS;
    constexpr int rotations = block.ROTATIONS;
    constexpr int kicks = block.KICK_PER_ROTATION;
    board_t<W, H> usable[orientations];
    std::array<board_t<W, H>, kicks> kicks2[orientations][rotations];
    static_for<orientations>([&](auto i) {
      usable[i] = usable_positions<block.minos[i]>(data);
    });
    static_for<orientations>([&](auto i) {
      static_for<rotations>([&](auto j) {
        constexpr auto target = block.rotation_target(i, j);
        kicks2[i][j] = kick_positions<block.kicks[i][j]>(usable[i], usable[target]);
      });
    });
    constexpr std::array<coord, 3> MOVES = {{{-1, 0}, {1, 0}, {0, -1}}};
    if (!usable[init_rot].template get<start[0], start[1]>()) [[unlikely]] {
      return {};
    }
    bool need_visit[orientations] = { };
    need_visit[init_rot] = true;
    std::array<board_t<W, H>, orientations> cache;
    cache[init_rot].template set<start[0], start[1]>();
    for (bool updated = true; updated;) [[unlikely]] {
      updated = false;
      static_for<orientations>([&](auto i){
        if (!need_visit[i]) {
          return;
        }
        need_visit[i] = false;
        for (bool updated2 = true; updated2;) [[likely]] {
          updated2 = false;
          static_for<MOVES.size()>([&](auto j){
            constexpr auto move = MOVES[j];
            board_t mask = usable[i] & ~cache[i];
            if (!mask.any()) {
              return;
            }
            board_t to = move_to_center<move, true>(cache[i]);
            to &= mask;
            if (to.any()) {
              cache[i] |= to;
              updated2 = true;
            }
          });
        }
        static_for<rotations>([&](auto j){
          constexpr int target = block.rotation_target(i, j);
          board_t<W, H> to;
          static_for<kicks>([&](auto k){
            board_t<W, H> from = cache[i] & kicks2[i][j][k];
            to |= move_to_center<block.kicks[i][j][k], true, false>(from);
          });
          board_t<W, H> det = to & ~cache[target];
          if (det.any()) {
            cache[target] |= to;
            need_visit[target] = true;
            if (target < i)
              updated = true;
          }
        });
      });
    }
    static_for<orientations>([&](auto i){
      cache[i] &= landable_positions(usable[i]);
    });
    return cache;
  }
  template <typename RS, coord start, unsigned init_rot=0, unsigned W, unsigned H>
  [[gnu::noinline]]
  constexpr void binary_bfs(board_t<W, H> *ret, const board_t<W, H> &data, char b) {
    call_with_block<RS>(b, [&]<block B>() {
      auto info = binary_bfs<B, start, init_rot>(data);
      for (std::size_t i = 0; i < info.size(); ++i) {
        ret[i] = info[i];
      }
    });
  }
  template <unsigned W, unsigned H>
  auto ordinary_bfs_without_binary(const board_t<W, H> &data, const auto &block, const coord &start, unsigned init_rot) {
    constexpr auto orientations = std::remove_cvref_t<decltype(block)>::ORIENTATIONS;
    bool my_data[H][W];
    static_for<H>([&](auto y) {
      static_for<W>([&](auto x) {
        my_data[y][x] = data.template get<x, y>();
      });
    });
    auto in_range = [](int x, int y) {
      return 0 <= x && unsigned(x) < W && 0 <= y && unsigned(y) < H;
    };
    auto usable = [&](int i, int x, int y) {
      for (const auto &[dx, dy] : block.minos[i]) {
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
    visit(init_rot, start[0], start[1]);
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
    std::array<board_t<W, H>, orientations> true_ret;
    static_for<orientations>([&](auto i) {
      static_for<H>([&](auto y) {
        static_for<W>([&](auto x) {
          if (ret[i][y][x]) {
            true_ret[i].template set<x, y>();
          }
        });
      });
    });
    return true_ret;
  }
  template <typename RS, unsigned W, unsigned H>
  [[gnu::noinline]]
  constexpr void ordinary_bfs_without_binary(board_t<W, H> *ret, const board_t<W, H> &data, char b, const coord &start, unsigned init_rot=0) {
    call_with_block<RS>(b, [&]<block B>() {
      auto info = ordinary_bfs_without_binary(data, B, start, init_rot);
      for (std::size_t i = 0; i < info.size(); ++i) {
        ret[i] = info[i];
      }
    });
  }
}