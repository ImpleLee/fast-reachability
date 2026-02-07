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
  template <Wrap<mino_p> auto mino, typename board_t>
  constexpr board_t usable_positions(board_t data) {
    board_t positions = ~board_t();
    static_for<std::tuple_size_v<decltype(mino)>>([&][[gnu::always_inline]](auto i) {
      constexpr auto move = mino[i];
      constexpr int x = move[0_szc], y = move[1_szc];
      if constexpr (y > 0) {
        positions &= (~data.template move<coord{0, -y}>()).template move<coord{-x, 0}>();
      } else {
        positions &= (~data).template move<-move>();
      }
    });
    return positions;
  }
  template <typename board_t>
  constexpr board_t landable_positions(board_t usable) {
    return usable & ~usable.template move<coord{0, 1}>();
  }
  template <typename board_t>
  constexpr board_t consecutive_lines(board_t usable) {
    const auto indicator01 = usable.get_heads();
    return indicator01.has_single_bit();
  }
  template <Wrap<mino_p> auto mino_from, Wrap<mino_p> auto mino_to, coord d, typename board_t>
  constexpr board_t move_usable(board_t data) {
    constexpr int dx = d[0_szc];
    constexpr bool need_mask = []{
      constexpr auto range_from = blocks::mino_range<mino_from>();
      constexpr auto range_to = blocks::mino_range<mino_to>();
      if constexpr (dx == 0) {
        return false;
      } else if constexpr (dx > 0) {
        return range_from[2] - dx - range_to[0] < 0;
      } else {
        return range_to[2] + dx - range_from[0] < 0;
      }
    }();
    return data.template move<d, need_mask>();
  }
  template <coord start, bool check_consecutive, typename board_t>
  constexpr board_t direct_reachable(board_t usable) {
    const auto current = usable & usable.template move<coord{0, -1}>();
    const auto covered = usable & ~current;
    const auto expandable = can_expand(current, covered);
    auto whole_line_usable = (expandable | ~covered.get_heads()).all_bits().populate_highest_bit();
    constexpr int removed_lines = board_t::height - start[1_szc];
    if constexpr (removed_lines > 0) {
      whole_line_usable |= ~(~board_t()).template move<coord{0, -removed_lines}>();
    }
    auto good_lines = whole_line_usable.remove_ones_after_zero();
    if constexpr (check_consecutive && removed_lines > 1) {
      good_lines &= (~board_t()).template move<coord{0, -(removed_lines - 1)}>();
    }
    if constexpr (check_consecutive) {
      const auto consecutive = consecutive_lines(usable);
      if (!consecutive.template get<start[1_szc]>()) {
        auto ret = board_t();
        ret.template set<start[0_szc], start[1_szc]>();
        return ret;
      }
    }
    return good_lines & usable;
  }
  template <block block, coord start, std::size_t init_rot, bool check_consecutive = true, typename board_t>
  constexpr std::array<board_t, block.shapes> binary_bfs(board_t data) {
    constexpr int orientations = block.orientations;
    constexpr int shapes = block.shapes;
    constexpr coord new_start{start[0_szc], std::min(start[1_szc], board_t::height - 1)};
    board_t usable[shapes];
    static_for<shapes>([&][[gnu::always_inline]](auto i) {
      usable[i] = usable_positions<block.minos[i]>(data);
    });
    constexpr std::array<coord, 3> MOVES = {{{-1, 0}, {1, 0}, {0, -1}}};
    constexpr coord start2 = new_start + block.mino_index[index_c<init_rot>][1_szc];
    constexpr auto init_rot2 = block.mino_index[index_c<init_rot>][0_szc];
    if (!usable[init_rot2].template get<start2[0_szc], start2[1_szc]>()) [[unlikely]] {
      return {};
    }
    std::array<bool, orientations> need_visit{};
    need_visit.fill(true);
    std::array<board_t, orientations> cache;
    static_for<orientations>([&][[gnu::always_inline]](auto i){
      constexpr coord this_start = new_start + block.mino_index[i][1_szc];
      constexpr auto rot = block.mino_index[i][0_szc];
      cache[i] = direct_reachable<this_start, check_consecutive>(usable[rot]);
    });
    const auto quick_check = [&][[gnu::always_inline]]() {
      bool found_all = true;
      std::array<board_t, shapes> ret;
      static_for<orientations>([&][[gnu::always_inline]](auto i){
        constexpr auto index = block.mino_index[i][0_szc];
        ret[index] |= cache[i];
      });
      static_for<shapes>([&][[gnu::always_inline]](auto i){
        const auto landable = landable_positions(usable[i]);
        found_all = found_all && ret[i].contains(landable);
        ret[i] &= landable;
      });
      return std::pair{found_all, ret};
    };
    for (bool updated = true; updated;) [[unlikely]] {
      auto [found_all, ret] = quick_check();
      if (found_all) return ret;
      updated = false;
      static_for<orientations>([&][[gnu::always_inline]](auto i){
        if (!need_visit[i]) {
          return;
        }
        constexpr auto index = index_c<block.mino_index[i][0_szc]>;
        need_visit[i] = false;
        while (true) {
          board_t result = cache[i];
          static_for<MOVES.size()>([&][[gnu::always_inline]](auto j) {
            result |= move_usable<block.minos[index], block.minos[index], MOVES[j]>(cache[i]);
          });
          result &= usable[index];
          if (cache[i].contains(result)) [[unlikely]] {
            break;
          }
          cache[i] = result;
        }
        static_for<std::tuple_size_v<decltype(block.kicks)>>([&][[gnu::always_inline]](auto j){
          constexpr auto this_kick = block.kicks[j];
          constexpr auto diff = this_kick[0_szc];
          constexpr auto kick_table = this_kick[1_szc];
          if constexpr (diff[0_szc] != i) {
            return;
          } else {
            constexpr auto target = index_c<diff[1_szc]>;
            static_assert(target != i);
            board_t to = cache[target];
            constexpr auto index2 = index_c<block.mino_index[target][0_szc]>;
            board_t temp = cache[i];
            static_for<std::tuple_size_v<decltype(kick_table)>>([&][[gnu::always_inline]](auto k){
              to |= move_usable<block.minos[index], block.minos[index2], kick_table[k]>(temp);
              temp &= ~move_usable<block.minos[index2], block.minos[index], -kick_table[k]>(usable[index2]);
            });
            to &= usable[index2];
            if (!cache[target].contains(to)) {
              need_visit[target] = true;
              if constexpr (target < i)
                updated = true;
            }
            cache[target] = to;
          }
        });
      });
    }
    auto [_, ret] = quick_check();
    return ret;
  }
  template <typename RS, coord start, unsigned init_rot=0, typename board_t>
  constexpr static_vector<board_t, 4> binary_bfs(board_t data, block_type b) {
    return call_with_block<RS>(b, [=]<block B>() {
      auto ret = binary_bfs<B, start, init_rot>(data);
      return static_vector<board_t, 4>{std::span{ret}};
    });
  }
}