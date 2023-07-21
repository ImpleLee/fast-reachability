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
  #define new_board
  #ifndef new_board
  #define DEF_OPERATOR(ret, op, param, expr) \
    constexpr ret operator op(param) const { \
      return {data op expr}; \
    } \
    constexpr ret &operator op##=(param) { \
      data op##= expr; \
      return *this; \
    }
  #define DEF_BOARD(name, other_name) \
    struct name { \
      constexpr name() { } \
      constexpr name(const string &s): data(s, 0, string::npos, ' ', 'X') { } \
      friend struct other_name; \
      constexpr name(const bitset<W * H> &d_): data(d_) { } \
      constexpr explicit name(const other_name &other): data((~other).data) { } \
      constexpr name set(int x, int y) { \
        assert(!(x < 0 || x >= W || y < 0 || y >= H)); \
        data.set(y * W + x); \
        return *this; \
      } \
      constexpr int get(int x, int y) const { \
        if (x < 0 || x >= W || y < 0 || y >= H) { \
          return 2; \
        } \
        return data[y * W + x]; \
      }\
      constexpr bool any() const { \
        return data.any(); \
      } \
      constexpr name operator~() const { \
        return {~data}; \
      } \
      constexpr bool operator[](size_t i) const { \
        return data[i]; \
      } \
      constexpr bool operator!=(const name &other) const { \
        return data != other.data; \
      } \
      DEF_OPERATOR(name, &, const name &rhs, rhs.data) \
      DEF_OPERATOR(name, |, const name &rhs, rhs.data) \
      DEF_OPERATOR(name, >>, size_t i, i) \
      DEF_OPERATOR(name, <<, size_t i, i) \
    private: \
      bitset<W * H> data; \
    }
  DEF_BOARD(board_t, inv_board_t);
  DEF_BOARD(inv_board_t, board_t);
  #undef DEF_BOARD
  #undef DEF_OPERATOR
  #else
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
      constexpr name & operator<<=(size_t i) { \
        for (size_t j = H2 - 1; j < H2; --j) { \
          data[j] <<= i; \
          if (j > 0) { \
            data[j] |= data[j - 1] >> (W2 - i); \
          } \
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
  #endif
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
  template <bool check = true, bool reverse = false, class board_t>
  static constexpr board_t move_to_center(board_t board, const coord &d) {
    auto dx = d[0], dy = d[1];
    if constexpr (reverse) {
      dx = -dx;
      dy = -dy;
    }
    board_t mask;
    if constexpr (check) {
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
    if (move < 0) {
      board <<= -move;
    } else {
      board >>= move;
    }
    if constexpr (check) {
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
  static constexpr array<inv_board_t, N> kick_positions(const inv_board_t &start, const inv_board_t &end, const array<coord, N> &kick) {
    array<inv_board_t, N> positions;
    for (size_t i = 0; i < N; ++i) {
      positions[i] = move_to_center(end, kick[i]) & start;
    }
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
  constexpr array<inv_board_t, 4> binary_bfs(const block &block, const coord &start) const {
    const array<inv_board_t, 4> usable = [&](){
      array<inv_board_t, 4> usable;
      for (int i = 0; i < 4; ++i) {
        usable[i] = usable_positions(block.minos[i]);
      }
      return usable;
    }();
    const array<array<array<inv_board_t, 5>, 3>, 4> kicks = [&](){
      array<array<array<inv_board_t, 5>, 3>, 4> kicks;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
          kicks[i][j] = kick_positions(usable[i], usable[(i + j + 1) % 4], block.kicks[i][j]);
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
    bool need_visit[4] = {true, false, false, false};
    for (bool updated = true; updated;) {
      updated = false;
      for (int i = 0; i < 4; ++i) {
        if (!need_visit[i]) {
          continue;
        }
        do {
          need_visit[i] = false;
          for (auto &move: MOVES) {
            inv_board_t mask = usable[i] & ~ret[i];
            if (!mask.any()) {
              continue;
            }
            inv_board_t to = move_to_center<true, true>(ret[i], move) & mask;
            if (to.any()) {
              ret[i] |= to;
              need_visit[i] = true;
              updated = true;
            }
          }
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
  array<inv_board_t, 4> ordinary_bfs(const block &block, const coord &start) const {
    const array<inv_board_t, 4> usable = [&](){
      array<inv_board_t, 4> usable;
      for (int i = 0; i < 4; ++i) {
        usable[i] = usable_positions(block.minos[i]);
      }
      return usable;
    }();
    const array<array<array<inv_board_t, 5>, 3>, 4> kicks = [&](){
      array<array<array<inv_board_t, 5>, 3>, 4> kicks;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
          kicks[i][j] = kick_positions(usable[i], usable[(i + j + 1) % 4], block.kicks[i][j]);
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
  array<array<kick, 3>, 4> common = {{
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
  block T = {
    {{
      {{{-1, 0}, {0, 0}, {1, 0}, {0, 1}}},  // 0
      {{{0, 1}, {0, 0}, {0, -1}, {1, 0}}},  // R
      {{{1, 0}, {0, 0}, {-1, 0}, {0, -1}}}, // 2
      {{{0, -1}, {0, 0}, {0, 1}, {-1, 0}}}  // L
    }},
    common
  };
  block Z = {
    {{
      {{{-1, 1}, {0, 1}, {0, 0}, {1, 0}}},   // 0
      {{{1, 1}, {1, 0}, {0, 0}, {0, -1}}},   // R
      {{{1, -1}, {0, -1}, {0, 0}, {-1, 0}}}, // 2
      {{{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}}  // L
    }},
    common
  };
  block S = {
    {{
      {{{1, 1}, {0, 1}, {0, 0}, {-1, 0}}},   // 0
      {{{1, -1}, {1, 0}, {0, 0}, {0, 1}}},   // R
      {{{-1, -1}, {0, -1}, {0, 0}, {1, 0}}}, // 2
      {{{-1, 1}, {-1, 0}, {0, 0}, {0, -1}}}  // L
    }},
    common
  };
  block J = {
    {{
      {{{-1, 1}, {-1, 0}, {0, 0}, {1, 0}}}, // 0
      {{{1, 1}, {0, 1}, {0, 0}, {0, -1}}},  // R
      {{{1, -1}, {1, 0}, {0, 0}, {-1, 0}}}, // 2
      {{{-1, -1}, {0, -1}, {0, 0}, {0, 1}}} // L
    }},
    common
  };
  block L = {
    {{
      {{{-1, 0}, {0, 0}, {1, 0}, {1, 1}}},  // 0
      {{{0, 1}, {0, 0}, {0, -1}, {1, -1}}}, // R
      {{{1, 0}, {0, 0}, {-1, 0}, {-1, -1}}},// 2
      {{{0, -1}, {0, 0}, {0, 1}, {-1, 1}}}  // L
    }},
    common
  };
  block O = {
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
  block I = {
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
  block blocks[] = {I, J, L, O, S, T, Z};
  string name = "IJLOSTZ";
  for (size_t i = 0; i < board_names.size(); ++i) {
    cout << "BOARD " << board_names[i] << endl;
    const auto &b = boards[i];
    for (int j = 0; j < 7; ++j) {
      cout << " BLOCK " << name[j] << endl;
      const auto &B = blocks[j];
      auto binary = b.binary_bfs(B, {4, 20});
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
      cout << "  ordinary: " << bench([&](){b.ordinary_bfs(B, {4, 20});}) << "ns" << endl;
      cout << "  true ord: " << bench([&](){b.ordinary_bfs_without_binary(B, {4, 20});}) << "ns" << endl;
    }
  }
}