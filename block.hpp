#pragma once
#include <array>
#include <utility>

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
  template <int shapes, int orientations, int rotations, int block_per_mino, int kick_per_rotation>
  struct block {
    using mino = std::array<coord, block_per_mino>;
    using kick = std::array<coord, kick_per_rotation>;
    static constexpr int SHAPES = shapes;
    static constexpr int ORIENTATIONS = orientations;
    static constexpr int ROTATIONS = rotations;
    static constexpr int BLOCK_PER_MINO = block_per_mino;
    static constexpr int KICK_PER_ROTATION = kick_per_rotation;
    [[no_unique_address]]
    std::array<mino, shapes> minos;
    [[no_unique_address]]
    std::array<std::array<kick, rotations>, orientations> kicks;
    [[no_unique_address]]
    std::array<int, orientations> mino_index;
    [[no_unique_address]]
    std::array<coord, orientations> mino_offset;
    static constexpr int rotation_target(int from, int rotation_num) {
      if constexpr (orientations == 4 && rotations == 2) {
        // cw & ccw
        return (from + rotation_num * 2 + 1) % orientations;
      }
      return (from + rotation_num + 1) % orientations;
    }
  };
}
namespace reachability::blocks {
  template <std::array mino>
  constexpr std::array<int, 4> mino_range() {
    static_assert(mino.size() >= 1);
    auto [min_x, min_y] = mino[0];
    auto [max_x, max_y] = mino[0];
    for (auto [x, y] : mino) {
      if (x < min_x) min_x = x;
      if (x > max_x) max_x = x;
      if (y < min_y) min_y = y;
      if (y > max_y) max_y = y;
    }
    return {min_x, min_y, max_x, max_y};
  }

  template <int block_per_mino, int orientations>
  struct pure_block {
    using mino = std::array<coord, block_per_mino>;
    std::array<mino, orientations> minos;
  };

  template <int shapes, int block_per_mino, int orientations>
  struct block_with_offset {
    using mino = std::array<coord, block_per_mino>;
    std::array<mino, shapes> minos;
    std::array<int, orientations> mino_index;
    std::array<coord, orientations> mino_offset;
  };
  template <int block_per_mino, int orientations>
  constexpr block_with_offset<orientations, block_per_mino, orientations> convert(const pure_block<block_per_mino, orientations> &b) {
    block_with_offset<orientations, block_per_mino, orientations> ret;
    for (int i = 0; i < orientations; ++i) {
      ret.minos[i] = b.minos[i];
      ret.mino_index[i] = i;
      ret.mino_offset[i] = {0, 0};
    }
    return ret;
  }

  template <int shapes, int block_per_mino, int orientations>
  constexpr block_with_offset<shapes, block_per_mino, orientations> convert(
    const pure_block<block_per_mino, shapes> &b,
    const std::array<int, orientations> &mino_index,
    std::array<coord, orientations> mino_offset
  ) {
    return {
      b.minos,
      mino_index,
      mino_offset
    };
  }

  template <int orientations, int rotations, int kick_per_rotation>
  struct pure_kick {
    using kick = std::array<coord, kick_per_rotation>;
    std::array<std::array<kick, rotations>, orientations> kicks;
  };

  constexpr coord operator+(const coord &co1, const coord &co2) {
    return {co1[0] + co2[0], co1[1] + co2[1]};
  }
  template <int shapes, int orientations, int rotations, int block_per_mino, int kick_per_rotation>
  constexpr block<shapes, orientations, rotations, block_per_mino, kick_per_rotation> operator+(
    const block_with_offset<shapes, block_per_mino, orientations> &b,
    const pure_kick<orientations, rotations, kick_per_rotation> &k
  ) {
    std::array<std::array<std::array<coord, kick_per_rotation>, rotations>, orientations> kicks;
    for (int i = 0; i < orientations; ++i) {
      for (int j = 0; j < rotations; ++j) {
        const auto target = block<shapes, orientations, rotations, block_per_mino, kick_per_rotation>::rotation_target(i, j);
        const auto offset = -b.mino_offset[i] + b.mino_offset[target];
        for (int l = 0; l < kick_per_rotation; ++l) {
          kicks[i][j][l] = k.kicks[i][j][l] + offset;
        }
      }
    }
    return {
      b.minos,
      kicks,
      b.mino_index,
      b.mino_offset
    };
  }

  inline constexpr pure_kick<1, 0, 0> no_rotation;

  inline constexpr pure_block<4, 4> T = {{{
    {{{-1, 0}, {0, 0}, {1, 0}, {0, 1}}},  // 0
    {{{0, 1}, {0, 0}, {0, -1}, {1, 0}}},  // R
    {{{1, 0}, {0, 0}, {-1, 0}, {0, -1}}}, // 2
    {{{0, -1}, {0, 0}, {0, 1}, {-1, 0}}}  // L
  }}};
  inline constexpr pure_block<4, 2> Z  = {{{
    {{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}},   // 0
    {{{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}}  // L
  }}};
  inline constexpr pure_block<4, 2> S = {{{
    {{{1, 1}, {0, 1}, {0, 0}, {-1, 0}}},   // 0
    {{{-1, 1}, {-1, 0}, {0, 0}, {0, -1}}}  // L
  }}};
  inline constexpr pure_block<4, 4> J = {{{
    {{{-1, 1}, {-1, 0}, {0, 0}, {1, 0}}}, // 0
    {{{1, 1}, {0, 1}, {0, 0}, {0, -1}}},  // R
    {{{1, -1}, {1, 0}, {0, 0}, {-1, 0}}}, // 2
    {{{-1, -1}, {0, -1}, {0, 0}, {0, 1}}} // L
  }}};
  inline constexpr pure_block<4, 4> L = {{{
    {{{-1, 0}, {0, 0}, {1, 0}, {1, 1}}},  // 0
    {{{0, 1}, {0, 0}, {0, -1}, {1, -1}}}, // R
    {{{1, 0}, {0, 0}, {-1, 0}, {-1, -1}}},// 2
    {{{0, -1}, {0, 0}, {0, 1}, {-1, 1}}}  // L
  }}};
  inline constexpr pure_block<4, 1> O = {{{
    {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}},
  }}};
  inline constexpr pure_block<4, 2> I = {{{
    {{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}},   // 0
    {{{0, 1}, {0, 2}, {0, 0}, {0, -1}}},   // L
  }}};

  struct SRS { // used as a namespace but usable as a template parameter
    static constexpr pure_kick<4, 2, 5> common_kick = {{{
      {{ // 0
        {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}},  // -> R
        {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}}      // -> L
      }},
      {{ // R
        {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}},      // -> 2
        {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}}       // -> 0
      }},
      {{ // 2
        {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}},     // -> L
        {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}   // -> R
      }},
      {{ // L
        {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},   // -> 0
        {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}}    // -> 2
      }}
    }}};
    static constexpr pure_kick<4, 2, 5> I_kick = {{{
      {{
        {{{1, 0}, {-1, 0}, {2, 0}, {-1, -1}, {2, 2}}},
        {{{0, -1}, {-1, -1}, {2, -1}, {-1, 1}, {2, -2}}},
      }},
      {{
        {{{0, -1}, {-1, -1}, {2, -1}, {-1, 1}, {2, -2}}},
        {{{-1, 0}, {1, 0}, {-2, 0}, {1, 1}, {-2, -2}}},
      }},
      {{
        {{{-1, 0}, {1, 0}, {-2, 0}, {1, 1}, {-2, -2}}},
        {{{0, 1}, {1, 1}, {-2, 1}, {1, -1}, {-2, 2}}},
      }},
      {{
        {{{0, 1}, {1, 1}, {-2, 1}, {1, -1}, {-2, 2}}},
        {{{1, 0}, {-1, 0}, {2, 0}, {-1, -1}, {2, 2}}},
      }}
    }}};
    static constexpr auto T = convert(blocks::T) + common_kick;
    static constexpr auto Z = convert<2, 4, 4>(blocks::Z, {{0, 1, 0, 1}}, {{
      {{0, 0}},
      {{1, 0}},
      {{0, -1}},
      {{0, 0}}
    }}) + common_kick;
    static constexpr auto S = convert<2, 4, 4>(blocks::S, {{0, 1, 0, 1}}, {{
      {{0, 0}},
      {{1, 0}},
      {{0, -1}},
      {{0, 0}}
    }}) + common_kick;
    static constexpr auto J = convert(blocks::J) + common_kick;
    static constexpr auto L = convert(blocks::L) + common_kick;
    static constexpr auto O = convert(blocks::O) + no_rotation;
    static constexpr auto I = convert<2, 4, 4>(blocks::I, {{0, 1, 0, 1}}, {{
      {{0, 0}},
      {{0, -1}},
      {{-1, 0}},
      {{0, 0}}
    }}) + I_kick;
    SRS() = delete;
  };
}
