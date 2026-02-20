#pragma once
#include <array>
#include <utility>
#include "utils.hpp"

namespace reachability {
  using coord = tuple<int, int>;
  constexpr coord operator-(const coord &co) {
    return {-co[0_szc], -co[1_szc]};
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
  constexpr auto mino_index_p = vec_of<
    tuple_of<
      type_of<int>,
      type_of<coord>>>;
  constexpr auto kicks_p = vec_of<
    tuple_of<
      tuple_of<
        type_of<int>,
        type_of<int>>,
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
    int min_x = mino[0_szc][0_szc], max_x = min_x;
    int min_y = mino[0_szc][1_szc], max_y = min_y;
    static_for<std::tuple_size_v<decltype(mino)>>([&](auto i){
      int x = mino[i][0_szc], y = mino[i][1_szc];
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
    tuple_array<tuple<int, coord>, std::tuple_size_v<minos_t>> mino_index;
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
    return {co1[0_szc] + co2[0_szc], co1[1_szc] + co2[1_szc]};
  }
  template <
    block_with_offset b,
    pure_kick k
  > constexpr auto combine = []{
    auto kicks = k.kicks;
    static_for<std::tuple_size_v<decltype(k.kicks)>>([&](auto i){
      auto &[_, kick_table] = kicks[i];
      constexpr auto diff = k.kicks[i][0_szc];
      const auto offset = -b.mino_index[index_c<diff[0_szc]>][1_szc] + b.mino_index[index_c<diff[1_szc]>][1_szc];
      static_for<std::tuple_size_v<decltype(kick_table)>>([&](auto j){
        kick_table[j] = kick_table[j] + offset;
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
    tuple{coord{0, 0}, coord{0, 1}, coord{0, 2}, coord{0, 3}},    // L
  }};

  struct SRS { // used as a namespace but usable as a template parameter
    static constexpr pure_kick common_kick{tuple{
      tuple{tuple{0, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},  // 0 -> R
      tuple{tuple{0, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},     // 0 -> L
      tuple{tuple{1, 2}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},      // R -> 2
      tuple{tuple{1, 0}, tuple{coord{0, 0}, coord{1, 0}, coord{1, -1}, coord{0, 2}, coord{1, 2}}},      // R -> 0
      tuple{tuple{2, 3}, tuple{coord{0, 0}, coord{1, 0}, coord{1, 1}, coord{0, -2}, coord{1, -2}}},     // 2 -> L
      tuple{tuple{2, 1}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, 1}, coord{0, -2}, coord{-1, -2}}},  // 2 -> R
      tuple{tuple{3, 0}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},   // L -> 0
      tuple{tuple{3, 2}, tuple{coord{0, 0}, coord{-1, 0}, coord{-1, -1}, coord{0, 2}, coord{-1, 2}}},   // L -> 2
    }};
    static constexpr pure_kick I_kick{tuple{
      tuple{tuple{0, 1}, tuple{coord{0, 0}, coord{-2, 0}, coord{+1, 0}, coord{-2, -1}, coord{+1, +2}}}, // 0 -> R
      tuple{tuple{0, 3}, tuple{coord{0, 0}, coord{-1, 0}, coord{+2, 0}, coord{-1, +2}, coord{+2, -1}}}, // 0 -> L
      tuple{tuple{1, 2}, tuple{coord{0, 0}, coord{-1, 0}, coord{+2, 0}, coord{-1, +2}, coord{+2, -1}}}, // R -> 2
      tuple{tuple{1, 0}, tuple{coord{0, 0}, coord{+2, 0}, coord{-1, 0}, coord{+2, +1}, coord{-1, -2}}}, // R -> 0
      tuple{tuple{2, 3}, tuple{coord{0, 0}, coord{+2, 0}, coord{-1, 0}, coord{+2, +1}, coord{-1, -2}}}, // 2 -> L
      tuple{tuple{2, 1}, tuple{coord{0, 0}, coord{+1, 0}, coord{-2, 0}, coord{+1, -2}, coord{-2, +1}}}, // 2 -> R
      tuple{tuple{3, 0}, tuple{coord{0, 0}, coord{+1, 0}, coord{-2, 0}, coord{+1, -2}, coord{-2, +1}}}, // L -> 0
      tuple{tuple{3, 2}, tuple{coord{0, 0}, coord{-2, 0}, coord{+1, 0}, coord{-2, -1}, coord{+1, +2}}}, // L -> 2
    }};
    static constexpr auto T = combine<convert(blocks::T), common_kick>;
    static constexpr auto Z = combine<block_with_offset{blocks::Z.minos, tuple{
      tuple{0, coord{0, 0}},
      tuple{1, coord{1, 0}},
      tuple{0, coord{0, -1}},
      tuple{1, coord{0, 0}},
    }}, common_kick>;
    static constexpr auto S = combine<block_with_offset{blocks::S.minos, tuple{
      tuple{0, coord{0, 0}},
      tuple{1, coord{1, 0}},
      tuple{0, coord{0, -1}},
      tuple{1, coord{0, 0}},
    }}, common_kick>;
    static constexpr auto J = combine<convert(blocks::J), common_kick>;
    static constexpr auto L = combine<convert(blocks::L), common_kick>;
    static constexpr auto O = combine<convert(blocks::O), no_rotation>;
    static constexpr auto I = combine<block_with_offset{blocks::I.minos, tuple{
      tuple{0, coord{0, 0}},
      tuple{1, coord{1, -2}},
      tuple{0, coord{0, -1}},
      tuple{1, coord{0, -2}},
    }}, I_kick>;
    SRS() = delete;
  };
}
