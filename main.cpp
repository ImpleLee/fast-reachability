#include <bitset>
#include <array>
#include <string>
#include <vector>
#include <deque>
#include <iostream>
#include <queue>
#include <tuple>
#include <cassert>
#include <chrono>
using namespace std;

#define $ if (true)

using coord = array<signed char, 2>;
using mino = array<coord, 4>;
using kick = array<coord, 5>;

struct block {
  array<mino, 4> minos;
  array<array<kick, 3>, 4> kicks;
};

template <char name>
static constexpr block blocks = {};

constexpr array<array<kick, 3>, 4> common = {{
  {{ // 0
    {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}},  // -> R
    {{{0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}}},       // -> 2
    {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}}      // -> L
  }},
  {{ // R
    {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}},      // -> 2
    {{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}},       // -> L
    {{{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}}       // -> 0
  }},
  {{ // 2
    {{{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}},     // -> L
    {{{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}}},  // -> 0
    {{{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}}   // -> R
  }},
  {{ // L
    {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}},   // -> 0
    {{{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}},  // -> R
    {{{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}}    // -> 2
  }}
}};

template <>
constexpr block blocks<'T'> = {
  {{
    {{{-1, 0}, {0, 0}, {1, 0}, {0, 1}}},  // 0
    {{{0, 1}, {0, 0}, {0, -1}, {1, 0}}},  // R
    {{{1, 0}, {0, 0}, {-1, 0}, {0, -1}}}, // 2
    {{{0, -1}, {0, 0}, {0, 1}, {-1, 0}}}  // L
  }},
  common
};
template <>
constexpr block blocks<'Z'> = {
  {{
    {{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}},   // 0
    {{{1, 1}, {1, 0}, {0, 0}, {0, -1}}},   // R
    {{{1, -1}, {0, -1}, {0, 0}, {-1, 0}}}, // 2
    {{{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}}  // L
  }},
  common
};
template <>
constexpr block blocks<'S'> = {
  {{
    {{{1, 1}, {0, 1}, {0, 0}, {-1, 0}}},   // 0
    {{{1, -1}, {1, 0}, {0, 0}, {0, 1}}},   // R
    {{{-1, -1}, {0, -1}, {0, 0}, {1, 0}}}, // 2
    {{{-1, 1}, {-1, 0}, {0, 0}, {0, -1}}}  // L
  }},
  common
};
template <>
constexpr block blocks<'J'> = {
  {{
    {{{-1, 1}, {-1, 0}, {0, 0}, {1, 0}}}, // 0
    {{{1, 1}, {0, 1}, {0, 0}, {0, -1}}},  // R
    {{{1, -1}, {1, 0}, {0, 0}, {-1, 0}}}, // 2
    {{{-1, -1}, {0, -1}, {0, 0}, {0, 1}}} // L
  }},
  common
};
template <>
constexpr block blocks<'L'> = {
  {{
    {{{-1, 0}, {0, 0}, {1, 0}, {1, 1}}},  // 0
    {{{0, 1}, {0, 0}, {0, -1}, {1, -1}}}, // R
    {{{1, 0}, {0, 0}, {-1, 0}, {-1, -1}}},// 2
    {{{0, -1}, {0, 0}, {0, 1}, {-1, 1}}}  // L
  }},
  common
};
template <>
constexpr block blocks<'O'> = {
  {{
    {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}, // 0
    {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}, // R
    {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}, // 2
    {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}}  // L
  }},
  {{
    {{
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
    }},
    {{
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
    }},
    {{
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
    }},
    {{
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
      {{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}},
    }}
  }}
};
template <>
constexpr block blocks<'I'> = {
  {{
    {{{-1, 0}, {0, 0}, {1, 0}, {2, 0}}},   // 0
    {{{0, 0}, {0, 1}, {0, -1}, {0, -2}}},  // R
    {{{-1, 0}, {0, 0}, {1, 0}, {-2, 0}}},  // 2
    {{{0, 1}, {0, 2}, {0, 0}, {0, -1}}},   // L
  }},
  {{
    {{
      {{{1, 0}, {-1, 0}, {2, 0}, {-1, -1}, {2, 2}}},
      {{{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}},
      {{{0, -1}, {-1, -1}, {2, -1}, {-1, 1}, {2, -2}}},
    }},
    {{
      {{{0, -1}, {-1, -1}, {2, -1}, {-1, 1}, {2, -2}}},
      {{{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}}},
      {{{-1, 0}, {1, 0}, {-2, 0}, {1, 1}, {-2, -2}}},
    }},
    {{
      {{{-1, 0}, {1, 0}, {-2, 0}, {1, 1}, {-2, -2}}},
      {{{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}}},
      {{{0, 1}, {1, 1}, {-2, 1}, {1, -1}, {-2, 2}}},
    }},
    {{
      {{{0, 1}, {1, 1}, {-2, 1}, {1, -1}, {-2, 2}}},
      {{{0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}}},
      {{{1, 0}, {-1, 0}, {2, 0}, {-1, -1}, {2, 2}}},
    }}
  }}
};

static constexpr block get_block(char name) {
  switch (name) {
    case 'T': return blocks<'T'>;
    case 'Z': return blocks<'Z'>;
    case 'S': return blocks<'S'>;
    case 'J': return blocks<'J'>;
    case 'L': return blocks<'L'>;
    case 'O': return blocks<'O'>;
    case 'I': return blocks<'I'>;
    default: assert(false);
  }
}

template<typename F, std::size_t... S>
constexpr void static_for(F&& function, std::index_sequence<S...>) {
    int unpack[] = {0,
        (void(function(std::integral_constant<std::size_t, S>{})), 0)...
    };

    (void) unpack;
}

template<std::size_t iterations, typename F>
constexpr void static_for(F&& function) {
    static_for(std::forward<F>(function), std::make_index_sequence<iterations>());
}

template <typename board_t, int W, int H>
// should be constexpr
// but constexpr-bitset requires C++23
const board_t L = []() constexpr {
  board_t data;
  for (int i = 0; i < H; ++i) {
    data.set(W - 1, i);
  }
  return ~data;
}();
template <typename board_t, int W, int H>
// should be constexpr
// but constexpr-bitset requires C++23
const board_t R = []() constexpr {
  board_t data;
  for (int i = 0; i < H; ++i) {
    data.set(0, i);
  }
  return ~data;
}();

template <int W, int H>
struct board {
  struct inv_board_t;
  struct board_t;
  static constexpr const size_t W2 = (64 / W) * W;
  static constexpr const size_t H2 = (H - 1) / (64 / W) + 1;
  static constexpr const size_t DIFF = H2 * W2 - H * W;
  #define DEF_DUAL_OPERATOR(ret, op, param, expr2) \
    constexpr ret operator op(param) const { \
      ret result = *this; \
      result op##= expr2; \
      return result; \
    }
  #define DEF_OPERATOR(ret, op, param, expr, expr2) \
    constexpr ret &operator op##=(param) { \
      for (size_t i = 0; i < H2; ++i) { \
        data[i] op##= expr; \
      } \
      return *this; \
    } \
    DEF_DUAL_OPERATOR(ret, op, param, expr2)
  #define DEF_BOARD(name, name2) \
    struct name { \
      friend struct name2; \
      constexpr name() { } \
      constexpr name(const string &s) { \
        for (size_t i = 0; i + 1 < H2; ++i) { \
          data[i] = bitset<W2>{s, W * H - (i + 1) * W2, W2, ' ', 'X'}; \
        } \
        data[H2 - 1] = bitset<W2>{s, 0, W2 - DIFF, ' ', 'X'}; \
      } \
      constexpr explicit name(const name2 &other) { \
        for (size_t i = 0; i < H2; ++i) \
          data[i] = ~other.data[i]; \
      } \
      constexpr name operator~() const { \
        name other; \
        for (size_t i = 0; i < H2; ++i) { \
          other.data[i] = ~data[i]; \
        } \
        return other; \
      } \
      constexpr void set(int x, int y) { \
        data[y / (64 / W)].set((y % (64 / W)) * W + x); \
      } \
      constexpr int get(int x, int y) const { \
        if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) { \
          return 2; \
        } \
        return data[y / (64 / W)][(y % (64 / W)) * W + x]; \
      } \
      constexpr bool any() const { \
        for (size_t i = 0; i + 1 < H2; ++i) { \
          if (data[i].any()) { \
            return true; \
          } \
        } \
        if (((data[H2 - 1] << DIFF) >> DIFF).any()) { \
          return true; \
        } \
        return false; \
      } \
      constexpr bool operator!=(const name &other) const { \
        for (size_t i = 0; i + 1 < H2; ++i) { \
          if (data[i] != other.data[i]) { \
            return true; \
          } \
        } \
        if (((data[H2 - 1] << DIFF) >> DIFF) != ((other.data[H2 - 1] << DIFF) >> DIFF)) { \
          return true; \
        } \
        return false; \
      } \
      DEF_OPERATOR(name, &, const name &rhs, rhs.data[i], rhs) \
      DEF_OPERATOR(name, |, const name &rhs, rhs.data[i], rhs) \
      constexpr name & operator>>=(size_t i) { \
        data[H2 - 1] <<= DIFF; \
        data[H2 - 1] >>= DIFF; \
        for (size_t j = 0; j < H2; ++j) { \
          data[j] >>= i; \
          if (j + 1 < H2) { \
            data[j] |= data[j + 1] << (W2 - i); \
          } \
        } \
        return *this; \
      } \
      constexpr name & right_shift(size_t i) { \
        data[H2 - 1] <<= DIFF; \
        data[H2 - 1] >>= DIFF; \
        for (size_t j = 0; j < H2; ++j) { \
          data[j] >>= i; \
        } \
        return *this; \
      } \
      constexpr name & operator<<=(size_t i) { \
        for (size_t j = H2 - 1; j < H2; --j) { \
          data[j] <<= i; \
          if (j > 0) { \
            data[j] |= data[j - 1] >> (W2 - i); \
          } \
        } \
        return *this; \
      } \
      constexpr name & left_shift(size_t i) { \
        for (size_t j = H2 - 1; j < H2; --j) { \
          data[j] <<= i; \
        } \
        return *this; \
      } \
      DEF_DUAL_OPERATOR(name, >>, size_t j, j) \
      DEF_DUAL_OPERATOR(name, <<, size_t j, j) \
    private: \
      array<bitset<W2>, H2> data; \
    }
  DEF_BOARD(board_t, inv_board_t);
  DEF_BOARD(inv_board_t, board_t);
  #undef DEF_BOARD
  friend string to_string(const board_t &board) {
    string ret;
    for (int y = H - 1; y >= 0; --y) {
      for (int x = 0; x < W; ++x) {
        ret += board.get(x, y) ? "[]" : "  ";
      }
      ret += '\n';
    }
    return ret;
  }
  friend string to_string(const inv_board_t &board1, const board_t &board2) {
    string ret;
    for (int y = H - 1; y >= 0; --y) {
      for (int x = 0; x < W; ++x) {
        bool b1 = board1.get(x, y);
        bool b2 = board2.get(x, y);
        if (b1 && b2) {
          ret += "%%";
        } else if (b1) {
          ret += "..";
        } else if (b2) {
          ret += "[]";
        } else {
          ret += "  ";
        }
      }
      ret += '\n';
    }
    return ret;
  }
  friend string to_string(const inv_board_t &board1, const inv_board_t &board2, const board_t &board_3) {
    string ret;
    for (int y = H - 1; y >= 0; --y) {
      for (int x = 0; x < W; ++x) {
        bool tested[2] = {bool(board1.get(x, y)), bool(board2.get(x, y))};
        bool b3 = board_3.get(x, y);
        string symbols = "  <>[]%%";
        for (int i = 0; i < 2; ++i) {
          ret += symbols[b3 * 4 + tested[i] * 2 + i];
        }
      }
      ret += '\n';
    }
    return ret;
  }
  board_t data;
  template <class T>
  constexpr T remove_range(const T &data, unsigned start, unsigned end) {
    T below = data << (H * W - start);
    below >>= (H * W - start);
    T above = data >> end;
    above <<= start;
    return below | above;
  }
  constexpr int clear_full_lines() {
    auto board = data;
    int i = 1;
    for (; (W >> 1) >= i; i <<= 1) {
      board &= board >> i;
    }
    board &= board >> (W - i);
    int lines = 0;
    for (int y = 0; y < H; ++y) {
      if (board.get(0, y)) {
        data = remove_range(data, (y - lines) * W, (y - lines + 1) * W);
        ++lines;
      }
    }
    return lines;
  }
  // TODO: finally move d to template parameter and remove redundant template parameters
  template <bool check = true, bool reverse = false, bool use_shift = false, class board_t>
  static constexpr board_t move_to_center(board_t board, const coord &d) {
    auto dx = d[0], dy = d[1];
    if (reverse) {
      dx = -dx;
      dy = -dy;
    }
    board_t mask;
    if (check && dx != 0) {
      mask = ~board_t();
      if (dx < 0) {
        for (int i = 0; i < -dx; ++i) {
          mask &= R<board_t, W, H> << i;
        }
      } else if (dx > 0) {
        for (int i = 0; i < dx; ++i) {
          mask &= L<board_t, W, H> >> i;
        }
      }
    }
    int move = dy * W + dx;
    if (use_shift || dy == 0) {
      if (move < 0) {
        board.left_shift(-move);
      } else {
        board.right_shift(move);
      }
    } else {
      if (move < 0) {
        board <<= -move;
      } else {
        board >>= move;
      }
    }
    if (check && dx != 0) {
      board &= mask;
    }
    return board;
  }
  template <coord d, bool reverse = false, bool check = true, class board_t>
  static constexpr board_t move_to_center(board_t board) {
    auto dx = d[0], dy = d[1];
    if (reverse) {
      dx = -dx;
      dy = -dy;
    }
    board_t mask;
    if (check && dx != 0) {
      mask = ~board_t();
      if (dx < 0) {
        for (int i = 0; i < -dx; ++i) {
          mask &= R<board_t, W, H> << i;
        }
      } else if (dx > 0) {
        for (int i = 0; i < dx; ++i) {
          mask &= L<board_t, W, H> >> i;
        }
      }
    }
    int move = dy * W + dx;
    if (dy == 0) {
      if (move < 0) {
        board.left_shift(-move);
      } else {
        board.right_shift(move);
      }
    } else {
      if (move < 0) {
        board <<= -move;
      } else {
        board >>= move;
      }
    }
    if (check && dx != 0) {
      board &= mask;
    }
    return board;
  }
  template <size_t N>
  constexpr inv_board_t usable_positions(const array<coord, N> &mino) const {
    inv_board_t positions = ~inv_board_t();
    for (const auto &coord : mino) {
      positions &= move_to_center(inv_board_t{data}, coord);
    }
    return positions;
  }
  template <size_t N, array<coord, N> mino>
  constexpr inv_board_t usable_positions() const {
    inv_board_t positions = ~inv_board_t();
    for (const auto &coord : mino) {
      positions &= move_to_center(inv_board_t{data}, coord);
    }
    return positions;
  }
  template <size_t N>
  bool test_usable(const array<coord, N> &mino) const {
    auto usable = usable_positions(mino);
    bool ret = true;
    for (int x = 0; x < W; ++x) {
      for (int y = 0; y < H; ++y) {
        bool pred = usable.get(x, y);
        bool truth = true;
        for (const auto &[dx, dy] : mino) {
          truth = truth && !data.get(x + dx, y + dy);
        }
        if (pred != truth) {
          $ cout << "error at " << x << ", " << y << endl;
          ret = false;
        }
      }
    }
    return ret;
  }
  static constexpr inv_board_t landable_positions(const inv_board_t &usable) {
    return usable & ~(usable << W);
  }
  template <size_t N>
  bool test_landable(const array<coord, N> &mino) const {
    auto usable = usable_positions(mino);
    auto landable = landable_positions(usable);
    bool ret = true;
    for (int x = 0; x < W; ++x) {
      for (int y = 0; y < H; ++y) {
        bool pred = landable.get(x, y);
        if (!usable.get(x, y)) {
          continue;
        }
        bool truth = false;
        for (const auto &[dx, dy] : mino) {
          truth |= data.get(x + dx, y + dy - 1);
        }
        if (pred != truth) {
          $ cout << "error at " << x << ", " << y << endl;
          ret = false;
        }
      }
    }
    return ret;
  }
  template <size_t N>
  constexpr inv_board_t move_positions(const array<coord, N> &mino, const coord &dir) const {
    inv_board_t positions = usable_positions(mino);
    return move_to_center(positions, dir) & positions;
  }
  template <size_t N>
  bool test_move(const array<coord, N> &mino, const coord &dir) const {
    auto moved = move_positions(mino, dir);
    auto usable = usable_positions(mino);
    auto [dx, dy] = dir;
    bool ret = true;
    for (int x = 0; x < W; ++x) {
      for (int y = 0; y < H; ++y) {
        bool pred = moved.get(x, y);
        if (!usable.get(x, y)) {
          continue;
        }
        bool truth = usable.get(x + dx, y + dy) == 1;
        if (pred != truth) {
          $ cout << "error at " << x << ", " << y << endl;
          ret = false;
        }
      }
    }
    return ret;
  }
  template <size_t N>
  static constexpr array<inv_board_t, N> kick_positions(
    const inv_board_t &start, const inv_board_t &end,
    const array<inv_board_t, 5> &end_moved,
    const array<coord, N> &kick) {
    array<inv_board_t, N> positions;
    for (size_t i = 0; i < N; ++i) {
      if (kick[i][0] < -2 || kick[i][0] > 2) {
        positions[i] = move_to_center(end, kick[i]);
      } else {
        positions[i] = move_to_center<false>(end_moved[kick[i][0]+2], {0, kick[i][1]});
      }
      positions[i] &= start;
    }
    inv_board_t temp = positions[0];
    for (size_t i = 1; i < N; ++i) {
      positions[i] &= ~temp;
      temp |= positions[i];
    }
    return positions;
  }
  template <size_t N, array<coord, N> kick>
  static constexpr array<inv_board_t, N> kick_positions(
    const inv_board_t &start, const inv_board_t &end,
    const array<inv_board_t, 5> &end_moved) {
    array<inv_board_t, N> positions;
    static_for<N>([&](auto i) {
      if constexpr (kick[i][0] < -2 || kick[i][0] > 2) {
        positions[i] = move_to_center<kick[i]>(end);
      } else {
        positions[i] = move_to_center<coord{0, kick[i][1]}>(end_moved[kick[i][0]+2]);
      }
      positions[i] &= start;
    });
    inv_board_t temp = positions[0];
    for (size_t i = 1; i < N; ++i) {
      positions[i] &= ~temp;
      temp |= positions[i];
    }
    return positions;
  }
  template <size_t N>
  static bool test_kick(const inv_board_t &start, const inv_board_t &end, const array<coord, N> &kick) {
    auto kicks = kick_positions(start, end, kick);
    bool ret = true;
    for (int x = 0; x < W; ++x) {
      for (int y = 0; y < H; ++y) {
        int pred = -1;
        for (size_t i = 0; i < N; ++i) {
          if (kicks[i].get(x, y)) {
            pred = i;
            break;
          }
        }
        int truth = -1;
        if (start.get(x, y) == 1)
          for (size_t i = 0; i < N; ++i) {
            if (end.get(x + kick[i][0], y + kick[i][1]) == 1) {
              truth = i;
              break;
            }
          }
        if (pred != truth) {
          $ cout << "error at " << x << ", " << y << ": should be " << truth << ", but got " << pred << endl;
          ret = false;
        }
      }
    }
    return ret;
  }
  template <bool use_optimize=false>
  [[gnu::noinline]]
  constexpr array<inv_board_t, 4> binary_bfs(const block &block, const coord &start) const {
    const array<inv_board_t, 4> usable = [&](){
      array<inv_board_t, 4> usable;
      for (int i = 0; i < 4; ++i) {
        usable[i] = usable_positions(block.minos[i]);
      }
      return usable;
    }();
    const array<array<inv_board_t, 5>, 4> usable_moved = [&](){
      array<array<inv_board_t, 5>, 4> usable_moved;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 5; ++j) {
          usable_moved[i][j] = move_to_center<true, false, true>(usable[i], {static_cast<signed char>(j-2), 0});
        }
      }
      return usable_moved;
    }();
    const array<array<array<inv_board_t, 5>, 3>, 4> kicks = [&](){
      array<array<array<inv_board_t, 5>, 3>, 4> kicks;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
          int target = (i + j + 1) % 4;
          kicks[i][j] = kick_positions(usable[i], usable[target], usable_moved[target], block.kicks[i][j]);
        }
      }
      return kicks;
    }();
    constexpr const coord LEFT_RIGHT[] = {{-1, 0}, {1, 0}};
    constexpr const coord DOWN = {0, -1};
    array<inv_board_t, 4> ret;
    if (!usable[0].get(start[0], start[1])) {
      return ret;
    }
    ret[0].set(start[0], start[1]);
    bool need_visit[4] = {true, false, false, false};
    for (bool updated = true; updated;) {
      updated = false;
      for (int i = 0; i < 4; ++i) {
        if (!need_visit[i]) {
          continue;
        }
        do {
          need_visit[i] = false;
          #define MOVE(move, param) { \
            inv_board_t mask = usable[i] & ~ret[i]; \
            if (mask.any()) { \
              inv_board_t to = move_to_center<param, true, param>(ret[i], move) & mask; \
              if (to.any()) { \
                ret[i] |= to; \
                need_visit[i] = true; \
                updated = true; \
              } \
            } \
          }
          for (auto &move: LEFT_RIGHT) {
            MOVE(move, true);
          }
          MOVE(DOWN, false);
          #undef MOVE
        } while (need_visit[i]);
        for (int j = 0; j < 3; ++j) {
          inv_board_t to;
          int target = (i + j + 1) % 4;
          for (int k = 0; k < 5; ++k) {
            inv_board_t from = ret[i] & kicks[i][j][k];
            to |= move_to_center<false, true>(from, block.kicks[i][j][k]);
          }
          inv_board_t det = to & ~ret[target];
          if (det.any()) {
            ret[target] |= to;
            need_visit[target] = true;
            updated = true;
          }
        }
      }
    }
    for (int i = 0; i < 4; ++i) {
      ret[i] &= landable_positions(usable[i]);
    }
    return ret;
  }
  template <block block, coord start, bool use_optimize=false>
  constexpr array<inv_board_t, 4> binary_bfs() const {
    const array<inv_board_t, 4> usable = [&](){
      array<inv_board_t, 4> usable;
      static_for<4>([&](auto i) {
        usable[i] = usable_positions<4, block.minos[i]>();
      });
      return usable;
    }();
    const array<array<inv_board_t, 5>, 4> usable_moved = [&](){
      array<array<inv_board_t, 5>, 4> usable_moved;
      static_for<4>([&](auto i) {
        static_for<5>([&](auto j) {
          usable_moved[i][j] = move_to_center<coord{int(j)-2, 0}>(usable[i]);
        });
      });
      return usable_moved;
    }();
    const array<array<array<inv_board_t, 5>, 3>, 4> kicks = [&](){
      array<array<array<inv_board_t, 5>, 3>, 4> kicks;
      static_for<4>([&](auto i) {
        static_for<3>([&](auto j) {
          constexpr auto target = (i + j + 1) % 4;
          kicks[i][j] = kick_positions<5, block.kicks[i][j]>(usable[i], usable[target], usable_moved[target]);
        });
      });
      return kicks;
    }();
    constexpr const coord MOVES[] = {{-1, 0}, {1, 0}, {0, -1}};
    array<inv_board_t, 4> ret;
    if (!usable[0].get(start[0], start[1])) {
      return ret;
    }
    ret[0].set(start[0], start[1]);
    bool need_visit[4] = {true, false, false, false};
    for (bool updated = true; updated;) {
      updated = false;
      static_for<4>([&](auto i){
        if (!need_visit[i]) {
          return;
        }
        do {
          need_visit[i] = false;
          static_for<3>([&](auto j){
            constexpr auto move = MOVES[j];
            inv_board_t mask = usable[i] & ~ret[i];
            if (!mask.any()) {
              return;
            }
            inv_board_t to = move_to_center<move, true>(ret[i]);
            to &= mask;
            if (to.any()) {
              ret[i] |= to;
              need_visit[i] = true;
              updated = true;
            }
          });
        } while (need_visit[i]);
        static_for<3>([&](auto j){
          constexpr int target = (i + j + 1) % 4;
          inv_board_t to;
          static_for<5>([&](auto k){
            inv_board_t from = ret[i] & kicks[i][j][k];
            to |= move_to_center<block.kicks[i][j][k], true, false>(from);
          });
          inv_board_t det = to & ~ret[target];
          if (det.any()) {
            ret[target] |= to;
            need_visit[target] = true;
            updated = true;
          }
        });
      });
    }
    for (int i = 0; i < 4; ++i) {
      ret[i] &= landable_positions(usable[i]);
    }
    return ret;
  }
  template <coord start, bool use_optimize=false>
  [[gnu::noinline]]
  constexpr array<inv_board_t, 4> binary_bfs(char block) const {
    switch (block) {
      case 'T': return binary_bfs<blocks<'T'>, start, use_optimize>();
      case 'Z': return binary_bfs<blocks<'Z'>, start, use_optimize>();
      case 'S': return binary_bfs<blocks<'S'>, start, use_optimize>();
      case 'J': return binary_bfs<blocks<'J'>, start, use_optimize>();
      case 'L': return binary_bfs<blocks<'L'>, start, use_optimize>();
      case 'O': return binary_bfs<blocks<'O'>, start, use_optimize>();
      case 'I': return binary_bfs<blocks<'I'>, start, use_optimize>();
      default: assert(false);
    }
  }
  array<inv_board_t, 4> ordinary_bfs(const block &block, const coord &start) const {
    const array<inv_board_t, 4> usable = [&](){
      array<inv_board_t, 4> usable;
      for (int i = 0; i < 4; ++i) {
        usable[i] = usable_positions(block.minos[i]);
      }
      return usable;
    }();
    const array<array<inv_board_t, 5>, 4> usable_moved = [&](){
      array<array<inv_board_t, 5>, 4> usable_moved;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 5; ++j) {
          usable_moved[i][j] = move_to_center(usable[i], {static_cast<signed char>(j-2), 0});
        }
      }
      return usable_moved;
    }();
    const array<array<array<inv_board_t, 5>, 3>, 4> kicks = [&](){
      array<array<array<inv_board_t, 5>, 3>, 4> kicks;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
          int target = (i + j + 1) % 4;
          kicks[i][j] = kick_positions(usable[i], usable[target], usable_moved[target], block.kicks[i][j]);
        }
      }
      return kicks;
    }();
    constexpr const coord MOVES[] = {{0, -1}, {-1, 0}, {1, 0}};
    array<inv_board_t, 4> ret;
    if (!usable[0].get(start[0], start[1])) {
      return ret;
    }
    ret[0].set(start[0], start[1]);
    queue<tuple<int, int, int>> q;
    q.emplace(start[0], start[1], 0);
    while (!q.empty()) {
      const auto [x, y, i] = q.front();
      q.pop();
      for (auto &[dx, dy]: MOVES) {
        if (usable[i].get(x + dx, y + dy) && !ret[i].get(x + dx, y + dy)) {
          ret[i].set(x + dx, y + dy);
          q.emplace(x + dx, y + dy, i);
        }
      }
      for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 5; ++k) {
          auto &[dx, dy] = block.kicks[i][j][k];
          if (kicks[i][j][k].get(x, y) && !ret[(i + j + 1) % 4].get(x + dx, y + dy)) {
            ret[(i + j + 1) % 4].set(x + dx, y + dy);
            q.emplace(x + dx, y + dy, (i + j + 1) % 4);
            break;
          }
        }
      }
    }
    for (int i = 0; i < 4; ++i) {
      ret[i] &= landable_positions(usable[i]);
    }
    return ret;
  }
  array<inv_board_t, 4> ordinary_bfs_without_binary(const block &block, const coord &start) const {
    bool my_data[H][W];
    for (int y = 0; y < H; ++y) {
      for (int x = 0; x < W; ++x) {
        my_data[y][x] = data.get(x, y);
      }
    }
    auto in_range = [](int x, int y) {
      return 0 <= x && x < W && 0 <= y && y < H;
    };
    auto usable = [&](int i, int x, int y) {
      for (const auto &[dx, dy] : block.minos[i]) {
        if (!in_range(x + dx, y + dy)) return false;
        if (my_data[y + dy][x + dx]) return false;
      }
      return true;
    };
    constexpr const coord MOVES[] = {{0, -1}, {-1, 0}, {1, 0}};
    bool visited[4][H][W];
    for (int i = 0; i < 4; ++i) {
      for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
          visited[i][y][x] = false;
        }
      }
    }
    array<inv_board_t, 4> ret;
    if (!usable(0, start[0], start[1])) {
      return ret;
    }
    visited[0][start[1]][start[0]] = true;
    queue<tuple<int, int, int>> q;
    q.emplace(start[0], start[1], 0);
    while (!q.empty()) {
      const auto [x, y, i] = q.front();
      q.pop();
      for (auto &[dx, dy] : MOVES) {
        if (in_range(x + dx, y + dy) && !visited[i][y + dy][x + dx] && usable(i, x + dx, y + dy)) {
          visited[i][y + dy][x + dx] = true;
          q.emplace(x + dx, y + dy, i);
          if (!usable(i, x + dx, y + dy - 1))
            ret[i].set(x + dx, y + dy);
        }
      }
      for (int j = 0; j < 3; ++j) {
        const auto target = (i + j + 1) % 4;
        for (int k = 0; k < 5; ++k) {
          auto &[dx, dy] = block.kicks[i][j][k];
          if (in_range(x + dx, y + dy) && usable(target, x + dx, y + dy)) {
            if (!visited[target][y + dy][x + dx]) {
              visited[target][y + dy][x + dx] = true;
              q.emplace(x + dx, y + dy, target);
              if (!usable(target, x + dx, y + dy - 1))
                ret[target].set(x + dx, y + dy);
            }
            break;
          }
        }
      }
    }
    return ret;
  }
};

bool _ = ios::sync_with_stdio(false);

board<10, 22> str_to_board(deque<string> &&b_str) {
  while (b_str.size() < 22) {
    b_str.emplace_front(10, ' ');
  }
  string res;
  for (auto &s : b_str) {
    res.append(s.rbegin(), s.rend());
  }
  return {res};
}
int main() {
  auto bench = [&](auto f, int count=50000) {
    auto start = chrono::system_clock::now();
    for (int _ = 0; _ < count; ++_) {
      f();
    }
    auto end = chrono::system_clock::now();
    return double(chrono::duration_cast<chrono::nanoseconds>(end - start).count()) / count;
  };
  const vector<board<10, 22>> boards = {
    str_to_board({
      "X         ",
      "X         ",
      "XX      XX",
      "XXX    XXX",
      "XXXX   XXX",
      "XXXX  XXXX",
      "XXX   XXXX",
      "XXXX XXXXX"
    }),
    str_to_board({
      "        XX",
      "XXXX  XXXX",
      "XXX   XXXX",
      "XXX XXXXXX",
      "XXX  XXXXX",
      "XX   XXXXX",
      "XXX XXXXXX",
      "XXX XXXXXX",
      "XXXX XXXXX"
    }),
    str_to_board({
      "XXXXXXXX  ",
      "XXXXXXXX  ",
      "X         ",
      "X         ",
      "X  XXXXXXX",
      "X  XXXXXXX",
      "X        X",
      "X        X",
      "XXXXXXXX X",
      "XXXXXXXX X",
      "         X",
      "         X"
    }),
    str_to_board({
      "XXXX  XXX ",
      "XXXXX XXXX",
      "      X  X",
      "          ",
      "   X      ",
      "    XX X X",
      "XXX X     ",
      "XX  X    X",
      "X    X    ",
      "XX  XXXX X",
      "X    XX   ",
      "XX XXX    ",
      "       X  ",
      "      XXX ",
      "XX X      ",
      "X    X    ",
      "X   X    X",
      "X  XX X XX",
      "XX  XXXXXX",
    }),
    str_to_board({
      "   XX     ",
      "X XX    XX",
      "XXXX XXXXX",
      "XXXX   XXX",
      "XXX     XX",
      "XXXXXX XXX",
      "XXXX   XXX",
      "XXXXX XXXX",
      "XXXX XXXXX",
      "XXXX XXXXX"
    }),
  };
  vector<string> board_names = {
    "TSPIN", "DT", "BAD", "LZT", "4T"
  };
  string name = "IJLOSTZ";
  for (size_t i = 0; i < board_names.size(); ++i) {
    cout << "BOARD " << board_names[i] << endl;
    const auto &b = boards[i];
    for (int j = 0; j < 7; ++j) {
      cout << " BLOCK " << name[j] << endl;
      const auto &B = get_block(name[j]);
      auto binary = b.binary_bfs<coord{4, 20}>(name[j]);
      auto ordinary = b.ordinary_bfs(B, {4, 20});
      auto ordinary_without_binary = b.ordinary_bfs_without_binary(B, {4, 20});
      for (int i = 0; i < 4; ++i) {
        if (binary[i] != ordinary[i]) {
          cout << "  binary[" << i << "] != ordinary[" << i << "]" << endl;
          cout << to_string(binary[i], ordinary[i], b.data) << endl;
        }
        if (binary[i] != ordinary_without_binary[i]) {
          cout << "  binary[" << i << "] != ordinary_without_binary[" << i << "]" << endl;
          cout << to_string(binary[i], ordinary_without_binary[i], b.data) << endl;
        }
      }
      cout << "  binary  : " << bench([&](){b.binary_bfs(B, {4, 20});}) << "ns" << endl;
      cout << "  binary 2: " << bench([&](){b.binary_bfs<coord{4, 20}>(name[j]);}) << "ns" << endl;
      cout << "  ordinary: " << bench([&](){b.ordinary_bfs(B, {4, 20});}) << "ns" << endl;
      cout << "  true ord: " << bench([&](){b.ordinary_bfs_without_binary(B, {4, 20});}) << "ns" << endl;
    }
  }
}