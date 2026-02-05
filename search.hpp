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
      constexpr auto move = std::get<i>(mino);
      constexpr int x = move[0], y = move[1];
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
    constexpr int dx = d[0];
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
  template <block block, coord start, std::size_t init_rot, typename board_t>
  constexpr std::array<board_t, block.shapes> binary_bfs(board_t data) {
    constexpr int orientations = block.orientations;
    constexpr int shapes = block.shapes;
    board_t usable[shapes];
    static_for<shapes>([&][[gnu::always_inline]](auto i) {
      usable[i] = usable_positions<std::get<i>(block.minos)>(data);
    });
    constexpr std::array<coord, 3> MOVES = {{{-1, 0}, {1, 0}, {0, -1}}};
    constexpr coord start2 = start + std::get<1>(std::get<init_rot>(block.mino_index));
    constexpr auto init_rot2 = std::get<0>(std::get<init_rot>(block.mino_index));
    if (!usable[init_rot2].template get<start2[0], start2[1]>()) [[unlikely]] {
      return {};
    }
    bool need_visit[orientations] = { };
    need_visit[init_rot] = true;
    std::array<board_t, orientations> cache;
    const auto consecutive = consecutive_lines(usable[init_rot2]);
    if (consecutive.template get<start2[1]>()) [[likely]] {
      const auto current = usable[init_rot2] & usable[init_rot2].template move<coord{0, -1}>();
      const auto covered = usable[init_rot2] & ~current;
      const auto expandable = can_expand(current, covered);
      auto whole_line_usable = (expandable | ~covered.get_heads()).all_bits().populate_highest_bit();
      constexpr int removed_lines = board_t::height - start2[1];
      if constexpr (removed_lines > 0) {
        whole_line_usable |= ~(~board_t()).template move<coord{0, -removed_lines}>();
      }
      auto good_lines = whole_line_usable.remove_ones_after_zero();
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
        constexpr auto index = std::get<0>(std::get<i>(block.mino_index));
        need_visit[i] = false;
        while (true) {
          board_t result = cache[i];
          static_for<MOVES.size()>([&][[gnu::always_inline]](auto j) {
            result |= move_usable<std::get<index>(block.minos), std::get<index>(block.minos), MOVES[j]>(cache[i]);
          });
          result &= usable[index];
          if (cache[i].contains(result)) [[unlikely]] {
            break;
          }
          cache[i] = result;
        }
        static_for<std::tuple_size_v<decltype(block.kicks)>>([&][[gnu::always_inline]](auto j){
          constexpr auto this_kick = std::get<j>(block.kicks);
          constexpr auto diff = std::get<0>(this_kick);
          constexpr auto kick_table = std::get<1>(this_kick);
          if constexpr (std::get<0>(diff) != i) {
            return;
          }
          constexpr int target = std::get<1>(diff);
          board_t to = cache[target];
          constexpr auto index2 = std::get<0>(std::get<target>(block.mino_index)) ;
          board_t temp = cache[i];
          static_for<std::tuple_size_v<decltype(kick_table)>>([&][[gnu::always_inline]](auto k){
            to |= move_usable<std::get<index>(block.minos), std::get<index2>(block.minos), std::get<k>(kick_table)>(temp);
            temp &= ~move_usable<std::get<index2>(block.minos), std::get<index>(block.minos), -std::get<k>(kick_table)>(usable[index2]);
          });
          to &= usable[index2];
          if (!cache[target].contains(to)) {
            need_visit[target] = true;
            if constexpr (target < i)
              updated = true;
          }
          cache[target] = to;
        });
      });
    }
    std::array<board_t, shapes> ret;
    static_for<orientations>([&][[gnu::always_inline]](auto i){
      constexpr auto index = std::get<0>(std::get<i>(block.mino_index)) ;
      ret[index] |= cache[i];
    });
    static_for<shapes>([&][[gnu::always_inline]](auto i){
      ret[i] &= landable_positions(usable[i]);
    });
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