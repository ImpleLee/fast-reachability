#pragma once
#include <array>

namespace reachability {
  using coord = std::array<int, 2>;
  constexpr coord operator-(const coord &co) {
    return {-co[0], -co[1]};
  }
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
}
namespace reachability::blocks {
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

  inline constexpr pure_kick<1, 0, 0> no_rotation;

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
    {{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}},   // 0
    {{{0, 0}, {0, 1}, {0, -1}, {0, -2}}},  // R
    {{{-1, 0}, {0, 0}, {1, 0}, {-2, 0}}},  // 2
    {{{0, 1}, {0, 2}, {0, 0}, {0, -1}}},   // L
  }}};

  template <typename RS>
  inline constexpr auto call_with_block = [](){
    constexpr struct {} _;
    static_assert(std::is_same_v<RS, decltype(_)>, "unregistered rotation system");
  };

  struct SRS { // used as a namespace but usable as a template parameter
    static inline constexpr pure_kick<4, 2, 5> common_kick = {{{
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
    static inline constexpr pure_kick<4, 2, 5> I_kick = {{{
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
    static inline constexpr auto T = blocks::T + common_kick;
    static inline constexpr auto Z = blocks::Z + common_kick;
    static inline constexpr auto S = blocks::S + common_kick;
    static inline constexpr auto J = blocks::J + common_kick;
    static inline constexpr auto L = blocks::L + common_kick;
    static inline constexpr auto O = blocks::O + no_rotation;
    static inline constexpr auto I = blocks::I + I_kick;
  private:
    ~SRS();
  };

  template <>
  inline constexpr auto call_with_block<SRS> =
      []<typename F>(char ch, F f) {
    switch (ch) {
      case 'T': f.template operator()<SRS::T>(); return;
      case 'Z': f.template operator()<SRS::Z>(); return;
      case 'S': f.template operator()<SRS::S>(); return;
      case 'J': f.template operator()<SRS::J>(); return;
      case 'L': f.template operator()<SRS::L>(); return;
      case 'O': f.template operator()<SRS::O>(); return;
      case 'I': f.template operator()<SRS::I>(); return;
    }
  };
}