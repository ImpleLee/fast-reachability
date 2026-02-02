#pragma once
#include <array>
#include <utility>
#include "utils.hpp"

namespace reachability {
  using coord = std::array<int, 2>;
  constexpr coord operator-(const coord &co) {
    return {-co[0], -co[1]};
  }

  enum class block_type {
    T, Z, S, J, L, O, I
  };

  template <typename RS>
  [[gnu::always_inline]] constexpr auto call_with_block(block_type b, auto f) {
    using enum block_type;
    switch (b) {
      case T: return f.template operator()<RS::T>();
      case Z: return f.template operator()<RS::Z>();
      case S: return f.template operator()<RS::S>();
      case J: return f.template operator()<RS::J>();
      case L: return f.template operator()<RS::L>();
      case O: return f.template operator()<RS::O>();
      case I: return f.template operator()<RS::I>();
      default: std::unreachable();
    }
  }

  constexpr auto name_of(block_type b) {
    using enum block_type;
    switch (b) {
      case T: return 'T';
      case Z: return 'Z';
      case S: return 'S';
      case J: return 'J';
      case L: return 'L';
      case O: return 'O';
      case I: return 'I';
      default: std::unreachable();
    }
  }

  constexpr auto block_from_name(char c) {
    using enum block_type;
    switch (c) {
      case 'T': return T;
      case 'Z': return Z;
      case 'S': return S;
      case 'J': return J;
      case 'L': return L;
      case 'O': return O;
      case 'I': return I;
      default: std::unreachable();
    }
  }

  constexpr auto mino_p = vec_of<type_of<coord>>;
  constexpr auto minos_p = vec_of<mino_p>;
  constexpr auto mino_index_p = vec_of<type_of<std::pair<int, coord>>>;
  constexpr auto kicks_p = vec_of<
    tuple_of<
      type_of<std::pair<int, int>>,
      vec_of<
        type_of<coord>>>>;

  template <
    Wrap<minos_p> minos_t,
    Wrap<mino_index_p> mino_index_t,
    Wrap<kicks_p> kicks_t
  > struct block {
    static constexpr int shapes = std::tuple_size_v<minos_t>;
    static constexpr int orientations = std::tuple_size_v<mino_index_t>;
    minos_t minos;
    mino_index_t mino_index;
    kicks_t kicks;
  };
}
namespace reachability::blocks {
  template <Wrap<mino_p> auto mino>
  constexpr std::array<int, 4> mino_range() {
    static_assert(std::tuple_size_v<decltype(mino)> >= 1);
    auto [min_x, min_y] = std::get<0>(mino);
    auto [max_x, max_y] = std::get<0>(mino);
    static_for<std::tuple_size_v<decltype(mino)>>([&](auto i){
      auto [x, y] = std::get<i>(mino);
      if (x < min_x) min_x = x;
      if (x > max_x) max_x = x;
      if (y < min_y) min_y = y;
      if (y > max_y) max_y = y;
    });
    return {min_x, min_y, max_x, max_y};
  }

  template <Wrap<minos_p> minos_t>
  struct pure_block {
    minos_t minos;
  };

  template <
    Wrap<minos_p> minos_t,
    Wrap<mino_index_p> mino_index_t
  > struct block_with_offset {
    minos_t minos;
    mino_index_t mino_index;
  };

  template <Wrap<minos_p> minos_t>
  constexpr auto convert(const pure_block<minos_t> &b) {
    std::array<std::pair<int, coord>, std::tuple_size_v<minos_t>> mino_index;
    static_for<std::tuple_size_v<minos_t>>([&](auto i){
      mino_index[i] = {i, coord{0, 0}};
    });
    return block_with_offset{b.minos, mino_index};
  }

  template <Wrap<kicks_p> kicks_t>
  struct pure_kick {
    kicks_t kicks;
    constexpr explicit pure_kick(kicks_t kicks): kicks(kicks) {}
  };

  constexpr coord operator+(const coord &co1, const coord &co2) {
    return {co1[0] + co2[0], co1[1] + co2[1]};
  }
  template <
    block_with_offset b,
    pure_kick k
  > constexpr auto combine = []{
    auto kicks = k.kicks;
    static_for<std::tuple_size_v<decltype(k.kicks)>>([&](auto i){
      auto &[_, kick_table] = std::get<i>(kicks);
      constexpr auto diff = std::get<0>(std::get<i>(k.kicks));
      const auto offset = -std::get<std::get<0>(diff)>(b.mino_index).second + std::get<std::get<1>(diff)>(b.mino_index).second;
      static_for<std::tuple_size_v<decltype(kick_table)>>([&](auto j){
        std::get<j>(kick_table) = std::get<j>(kick_table) + offset;
      });
    });
    return block{b.minos, b.mino_index, kicks};
  }();

  inline constexpr pure_kick no_rotation{tuple{}};

  inline constexpr pure_block T = {tuple{
    tuple{coord{-1, 0}, coord{0, 0}, coord{1, 0}, coord{0, 1}},  // 0
    tuple{coord{0, 1}, coord{0, 0}, coord{0, -1}, coord{1, 0}},  // R
    tuple{coord{1, 0}, coord{0, 0}, coord{-1, 0}, coord{0, -1}}, // 2
    tuple{coord{0, -1}, coord{0, 0}, coord{0, 1}, coord{-1, 0}}  // L
  }};
  inline constexpr pure_block Z = {tuple{
    tuple{coord{-1, 1}, coord{0, 1}, coord{0, 0}, coord{1, 0}},   // 0
    tuple{coord{-1, -1}, coord{-1, 0}, coord{0, 0}, coord{0, 1}}  // L
  }};
  inline constexpr pure_block S = {tuple{
    tuple{coord{1, 1}, coord{0, 1}, coord{0, 0}, coord{-1, 0}},   // 0
    tuple{coord{-1, 1}, coord{-1, 0}, coord{0, 0}, coord{0, -1}}  // L
  }};
  inline constexpr pure_block J = {tuple{
    tuple{coord{-1, 1}, coord{-1, 0}, coord{0, 0}, coord{1, 0}},  // 0
    tuple{coord{1, 1}, coord{0, 1}, coord{0, 0}, coord{0, -1}},   // R
    tuple{coord{1, -1}, coord{1, 0}, coord{0, 0}, coord{-1, 0}},  // 2
    tuple{coord{-1, -1}, coord{0, -1}, coord{0, 0}, coord{0, 1}}  // L
  }};
  inline constexpr pure_block L = {tuple{
    tuple{coord{-1, 0}, coord{0, 0}, coord{1, 0}, coord{1, 1}},   // 0
    tuple{coord{0, 1}, coord{0, 0}, coord{0, -1}, coord{1, -1}},  // R
    tuple{coord{1, 0}, coord{0, 0}, coord{-1, 0}, coord{-1, -1}}, // 2
    tuple{coord{0, -1}, coord{0, 0}, coord{0, 1}, coord{-1, 1}}   // L
  }};
  inline constexpr pure_block O = {make_tuple(
    tuple{coord{0, 0}, coord{1, 0}, coord{0, 1}, coord{1, 1}}
  )};
  inline constexpr pure_block I = {tuple{
    tuple{coord{-1, 0}, coord{0, 0}, coord{1, 0}, coord{2, 0}},   // 0
    tuple{coord{0, 1}, coord{0, 2}, coord{0, 0}, coord{0, -1}},   // L
  }};

  struct SRS { // used as a namespace but usable as a template parameter
    static constexpr pure_kick common_kick{tuple{
      tuple{std::pair{0, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},  // 0 -> R
      tuple{std::pair{0, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},     // 0 -> L
      tuple{std::pair{1, 2}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},      // R -> 2
      tuple{std::pair{1, 0}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},      // R -> 0
      tuple{std::pair{2, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},     // 2 -> L
      tuple{std::pair{2, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},  // 2 -> R
      tuple{std::pair{3, 0}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},   // L -> 0
      tuple{std::pair{3, 2}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},   // L -> 2
    }};
    static constexpr pure_kick I_kick{tuple{
      tuple{std::pair{0, 1}, tuple{coord{1, 0}, coord{-1, 0}, coord{2, 0}, coord{-1, -1}, coord{2, 2}}},    // 0 -> R
      tuple{std::pair{0, 3}, tuple{coord{0, -1}, coord{-1, -1}, coord{2, -1}, coord{-1, 1}, coord{2, -2}}}, // 0 -> L
      tuple{std::pair{1, 2}, tuple{coord{0, -1}, coord{-1, -1}, coord{2, -1}, coord{-1, 1}, coord{2, -2}}}, // R -> 2
      tuple{std::pair{1, 0}, tuple{coord{-1, 0}, coord{1, 0}, coord{-2, 0}, coord{1, 1}, coord{-2, -2}}},   // R -> 0
      tuple{std::pair{2, 3}, tuple{coord{-1, 0}, coord{1, 0}, coord{-2, 0}, coord{1, 1}, coord{-2, -2}}},   // 2 -> L
      tuple{std::pair{2, 1}, tuple{coord{0, 1}, coord{1, 1}, coord{-2, 1}, coord{1, -1}, coord{-2, 2}}},    // 2 -> R
      tuple{std::pair{3, 0}, tuple{coord{0, 1}, coord{1, 1}, coord{-2, 1}, coord{1, -1}, coord{-2, 2}}},    // L -> 0
      tuple{std::pair{3, 2}, tuple{coord{1, 0}, coord{-1, 0}, coord{2, 0}, coord{-1, -1}, coord{2, 2}}},    // L -> 2
    }};
    static constexpr auto T = combine<convert(blocks::T), common_kick>;
    static constexpr auto Z = combine<block_with_offset{blocks::Z.minos, tuple{
      std::pair{0, coord{0, 0}},
      std::pair{1, coord{1, 0}},
      std::pair{0, coord{0, -1}},
      std::pair{1, coord{0, 0}},
    }}, common_kick>;
    static constexpr auto S = combine<block_with_offset{blocks::S.minos, tuple{
      std::pair{0, coord{0, 0}},
      std::pair{1, coord{1, 0}},
      std::pair{0, coord{0, -1}},
      std::pair{1, coord{0, 0}},
    }}, common_kick>;
    static constexpr auto J = combine<convert(blocks::J), common_kick>;
    static constexpr auto L = combine<convert(blocks::L), common_kick>;
    static constexpr auto O = combine<convert(blocks::O), no_rotation>;
    static constexpr auto I = combine<block_with_offset{blocks::I.minos, tuple{
      std::pair{0, coord{0, 0}},
      std::pair{1, coord{0, -1}},
      std::pair{0, coord{-1, 0}},
      std::pair{1, coord{0, 0}},
    }}, I_kick>;
    SRS() = delete;
  };
}
