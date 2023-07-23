#pragma once
#include <array>

namespace reachability {
  using coord = std::array<signed char, 2>;
}
namespace reachability::blocks {
  template <int orientations, int rotations, int block_per_mino, int kick_per_rotation>
  struct block {
    using mino = std::array<coord, 4>;
    using kick = std::array<coord, 5>;
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

  inline constexpr std::array<std::array<std::array<coord, 5>, 2>, 4> common = {{
    {{ // 0
      {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}},  // -> R
      // {{{0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}}},       // -> 2
      {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}}      // -> L
    }},
    {{ // R
      {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}},      // -> 2
      // {{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}},       // -> L
      {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}}       // -> 0
    }},
    {{ // 2
      {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}},     // -> L
      // {{{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}}},  // -> 0
      {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}   // -> R
    }},
    {{ // L
      {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},   // -> 0
      // {{{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}},  // -> R
      {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}}    // -> 2
    }}
  }};

  inline constexpr block<4, 2, 4, 5> T = {
    {{
      {{{-1, 0}, {0, 0}, {1, 0}, {0, 1}}},  // 0
      {{{0, 1}, {0, 0}, {0, -1}, {1, 0}}},  // R
      {{{1, 0}, {0, 0}, {-1, 0}, {0, -1}}}, // 2
      {{{0, -1}, {0, 0}, {0, 1}, {-1, 0}}}  // L
    }},
    common
  };
  inline constexpr block<4, 2, 4, 5> Z  = {
    {{
      {{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}},   // 0
      {{{1, 1}, {1, 0}, {0, 0}, {0, -1}}},   // R
      {{{1, -1}, {0, -1}, {0, 0}, {-1, 0}}}, // 2
      {{{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}}  // L
    }},
    common
  };
  inline constexpr block<4, 2, 4, 5> S = {
    {{
      {{{1, 1}, {0, 1}, {0, 0}, {-1, 0}}},   // 0
      {{{1, -1}, {1, 0}, {0, 0}, {0, 1}}},   // R
      {{{-1, -1}, {0, -1}, {0, 0}, {1, 0}}}, // 2
      {{{-1, 1}, {-1, 0}, {0, 0}, {0, -1}}}  // L
    }},
    common
  };
  inline constexpr block<4, 2, 4, 5> J = {
    {{
      {{{-1, 1}, {-1, 0}, {0, 0}, {1, 0}}}, // 0
      {{{1, 1}, {0, 1}, {0, 0}, {0, -1}}},  // R
      {{{1, -1}, {1, 0}, {0, 0}, {-1, 0}}}, // 2
      {{{-1, -1}, {0, -1}, {0, 0}, {0, 1}}} // L
    }},
    common
  };
  inline constexpr block<4, 2, 4, 5> L = {
    {{
      {{{-1, 0}, {0, 0}, {1, 0}, {1, 1}}},  // 0
      {{{0, 1}, {0, 0}, {0, -1}, {1, -1}}}, // R
      {{{1, 0}, {0, 0}, {-1, 0}, {-1, -1}}},// 2
      {{{0, -1}, {0, 0}, {0, 1}, {-1, 1}}}  // L
    }},
    common
  };
  inline constexpr block<1, 0, 4, 0> O = {
    {{
      {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}, // 0
    }},
    {}
  };
  inline constexpr block<4, 2, 4, 5> I = {
    {{
      {{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}},   // 0
      {{{0, 0}, {0, 1}, {0, -1}, {0, -2}}},  // R
      {{{-1, 0}, {0, 0}, {1, 0}, {-2, 0}}},  // 2
      {{{0, 1}, {0, 2}, {0, 0}, {0, -1}}},   // L
    }},
    {{
      {{
        {{{1, 0}, {-1, 0}, {2, 0}, {-1, -1}, {2, 2}}},
        // {{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}},
        {{{0, -1}, {-1, -1}, {2, -1}, {-1, 1}, {2, -2}}},
      }},
      {{
        {{{0, -1}, {-1, -1}, {2, -1}, {-1, 1}, {2, -2}}},
        // {{{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}}},
        {{{-1, 0}, {1, 0}, {-2, 0}, {1, 1}, {-2, -2}}},
      }},
      {{
        {{{-1, 0}, {1, 0}, {-2, 0}, {1, 1}, {-2, -2}}},
        // {{{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}},
        {{{0, 1}, {1, 1}, {-2, 1}, {1, -1}, {-2, 2}}},
      }},
      {{
        {{{0, 1}, {1, 1}, {-2, 1}, {1, -1}, {-2, 2}}},
        // {{{0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}}},
        {{{1, 0}, {-1, 0}, {2, 0}, {-1, -1}, {2, 2}}},
      }}
    }}
  };

  template <typename F>
  constexpr void call_with_block(char ch, F f) {
    switch (ch) {
      case 'T': f.template operator()<T>(); return;
      case 'Z': f.template operator()<Z>(); return;
      case 'S': f.template operator()<S>(); return;
      case 'J': f.template operator()<J>(); return;
      case 'L': f.template operator()<L>(); return;
      case 'O': f.template operator()<O>(); return;
      case 'I': f.template operator()<I>(); return;
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
namespace reachability {
  using blocks::block, blocks::call_with_block;
  using blocks::T, blocks::Z, blocks::S, blocks::J, blocks::L, blocks::O, blocks::I;
}