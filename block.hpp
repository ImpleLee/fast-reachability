#pragma once
#include <array>

namespace reachability {
  using coord = std::array<signed char, 2>;
}
namespace reachability::blocks {
  template <int orientations, int rotations, int block_per_mino, int kick_per_rotation>
  struct block {
    using mino = std::array<coord, block_per_mino>;
    using kick = std::array<coord, kick_per_rotation>;
    static constexpr int ORIENTATIONS = orientations;
    static constexpr int ROTATIONS = rotations;
    static constexpr int BLOCK_PER_MINO = block_per_mino;
    static constexpr int KICK_PER_ROTATION = kick_per_rotation;
    [[no_unique_address]]
    std::array<mino, orientations> minos;
    [[no_unique_address]]
    std::array<std::array<kick, rotations>, orientations> kicks;
    static constexpr int rotation_target(int from, int rotation_num) {
      if constexpr (orientations == 4 && rotations == 2) {
        // cw & ccw
        return (from + rotation_num * 2 + 1) % orientations;
      }
      return (from + rotation_num + 1) % orientations;
    }
  };

  template <int block_per_mino, int orientations>
  struct pure_block {
    using mino = std::array<coord, block_per_mino>;
    [[no_unique_address]]
    std::array<mino, orientations> minos;
  };

  template <int orientations, int rotations, int kick_per_rotation>
  struct pure_kick {
    using kick = std::array<coord, kick_per_rotation>;
    [[no_unique_address]]
    std::array<std::array<kick, rotations>, orientations> kicks;
  };

  template <int orientations, int rotations, int block_per_mino, int kick_per_rotation>
  constexpr block<orientations, rotations, block_per_mino, kick_per_rotation> operator+(
    const pure_block<block_per_mino, orientations> &b,
    const pure_kick<orientations, rotations, kick_per_rotation> &k
  ) {
    return {b.minos, k.kicks};
  }

  inline constexpr pure_kick<0, 0, 0> no_kick;
  template <int orientations, int block_per_mino>
  constexpr block<orientations, 0, block_per_mino, 0> operator+(
    const pure_block<block_per_mino, orientations> &b,
    const pure_kick<0, 0, 0> &
  ) {
    return {b.minos, {}};
  }

  inline constexpr pure_block<4, 4> T = {{{
    {{{-1, 0}, {0, 0}, {1, 0}, {0, 1}}},  // 0
    {{{0, 1}, {0, 0}, {0, -1}, {1, 0}}},  // R
    {{{1, 0}, {0, 0}, {-1, 0}, {0, -1}}}, // 2
    {{{0, -1}, {0, 0}, {0, 1}, {-1, 0}}}  // L
  }}};
  inline constexpr pure_block<4, 4> Z  = {{{
    {{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}},   // 0
    {{{1, 1}, {1, 0}, {0, 0}, {0, -1}}},   // R
    {{{1, -1}, {0, -1}, {0, 0}, {-1, 0}}}, // 2
    {{{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}}  // L
  }}};
  inline constexpr pure_block<4, 4> S = {{{
    {{{1, 1}, {0, 1}, {0, 0}, {-1, 0}}},   // 0
    {{{1, -1}, {1, 0}, {0, 0}, {0, 1}}},   // R
    {{{-1, -1}, {0, -1}, {0, 0}, {1, 0}}}, // 2
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
  inline constexpr pure_block<4, 4> I = {{{
    {{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}},    // 0
    {{{1, 0}, {1, 1}, {1, -1}, {1, -2}}},   // R
    {{{-1, -1}, {0, -1}, {1, -1}, {2, -1}}},// 2
    {{{0, 0}, {0, 1}, {0, -1}, {0, -2}}},   // L
  }}};

  namespace SRS {
    inline constexpr pure_kick<4, 2, 5> SRS_common_kick = {{{
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
    inline constexpr pure_kick<4, 2, 5> SRS_I_kick = {{{
      {{ // 0
        {{{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}}},    // -> R
        {{{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}}},    // -> L
      }},
      {{ // R
        {{{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}}},    // -> 2
        {{{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}}},    // -> 0
      }},
      {{ // 2
        {{{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}}},    // -> L
        {{{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}}},    // -> R
      }},
      {{ // L
        {{{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}}},    // -> 0
        {{{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}}},    // -> 2
      }}
    }}};
    template <typename F>
    constexpr void call_with_block(char ch, F f) {
      switch (ch) {
        case 'T': f.template operator()<T+SRS_common_kick>(); return;
        case 'Z': f.template operator()<Z+SRS_common_kick>(); return;
        case 'S': f.template operator()<S+SRS_common_kick>(); return;
        case 'J': f.template operator()<J+SRS_common_kick>(); return;
        case 'L': f.template operator()<L+SRS_common_kick>(); return;
        case 'O': f.template operator()<O+no_kick>(); return;
        case 'I': f.template operator()<I+SRS_I_kick>(); return;
      }
    }
    constexpr int get_orientations(char ch) {
      int ret = 0;
      call_with_block(ch, [&]<block b>() {
        ret = b.ORIENTATIONS;
      });
      return ret;
    }
  }
}
namespace reachability {
  using blocks::block, blocks::SRS::call_with_block, blocks::SRS::get_orientations;
}