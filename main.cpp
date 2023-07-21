#include <bitset>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <queue>
#include <tuple>
#include <cassert>
#include <chrono>
using namespace std;

#define $ if (true)

using coord = signed char[2];
using mino = coord[4];
using kick = coord[5];

struct block {
  mino minos[4];
  kick kicks[4][3];
};

template <size_t N>
constexpr bitset<N> remove_range(const bitset<N> &data, unsigned start, unsigned end) {
  bitset<N> below = data << (N - start);
  below >>= (N - start);
  bitset<N> above = data >> end;
  above <<= start;
  return below | above;
}

template <int W, int H>
struct board {
  #define DEF_OPERATOR(ret, op, param, expr) \
    ret operator op(param) const { \
      return {data op expr}; \
    } \
    ret &operator op##=(param) { \
      data op##= expr; \
      return *this; \
    }
  #define DEF_BOARD(name) \
    struct name { \
      bitset<W * H> data; \
      name set(size_t i, bool value = true) { \
        data.set(i, value); \
        return *this; \
      } \
      int get(int x, int y) const { \
        if (x < 0 || x >= W || y < 0 || y >= H) { \
          return 2; \
        } \
        return data[y * W + x]; \
      }\
      bool any() const { \
        return data.any(); \
      } \
      name operator~() const { \
        return {~data}; \
      } \
      bool operator[](size_t i) const { \
        return data[i]; \
      } \
      DEF_OPERATOR(name, &, const name &rhs, rhs.data) \
      DEF_OPERATOR(name, |, const name &rhs, rhs.data) \
      DEF_OPERATOR(name, >>, size_t i, i) \
      DEF_OPERATOR(name, <<, size_t i, i) \
    }
  DEF_BOARD(board_t);
  DEF_BOARD(inv_board_t);
  #undef DEF_BOARD
  #undef DEF_OPERATOR
  friend string to_string(const board_t &board) {
    string ret;
    for (int y = H - 1; y >= 0; --y) {
      for (int x = 0; x < W; ++x) {
        ret += board.data[y * W + x] ? "[]" : "  ";
      }
      ret += '\n';
    }
    return ret;
  }
  friend string to_string(const inv_board_t &board1, const board_t &board2) {
    string ret;
    for (int y = H - 1; y >= 0; --y) {
      for (int x = 0; x < W; ++x) {
        bool b1 = board1.data[y * W + x];
        bool b2 = board2.data[y * W + x];
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
        bool tested[2] = {board1[y * W + x], board2[y * W + x]};
        bool b3 = board_3.data[y * W + x];
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
  constexpr int clear_full_lines() {
    auto board = data;
    int i = 1;
    for (; (W >> 1) >= i; i <<= 1) {
      board &= board >> i;
    }
    board &= board >> (W - i);
    int lines = 0;
    for (int y = 0; y < H; ++y) {
      if (board[y * W]) {
        data = {remove_range(data.data, (y - lines) * W, (y - lines + 1) * W)};
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
        const board_t R = [](){
          board_t data;
          for (int i = 0; i < H; ++i) {
            data.set(i * W);
          }
          return ~data;
        }();
        for (int i = 0; i < -dx; ++i) {
          mask &= R >> i;
        }
      } else if (dx > 0) {
        const board_t L = [](){
          board_t data;
          for (int i = 0; i < H; ++i) {
            data.set(i * W + W - 1);
          }
          return ~data;
        }();
        for (int i = 0; i < dx; ++i) {
          mask &= L << i;
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
  constexpr inv_board_t usable_positions(const coord (&mino)[N]) const {
    inv_board_t positions = ~inv_board_t();
    for (const auto &coord : mino) {
      positions &= move_to_center(inv_board_t{~data.data}, coord);
    }
    return positions;
  }
  template <size_t N>
  bool test_usable(const coord (&mino)[N]) const {
    auto usable = usable_positions(mino);
    bool ret = true;
    for (int x = 0; x < W; ++x) {
      for (int y = 0; y < H; ++y) {
        bool pred = usable.get(x, y);
        bool truth = true;
        for (const auto &[dx, dy] : mino) {
          truth &= !data.get(x + dx, y + dy);
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
  bool test_landable(const coord (&mino)[N]) const {
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
  constexpr inv_board_t move_positions(const coord (&mino)[N], const coord &dir) const {
    inv_board_t positions = usable_positions(mino);
    return move_to_center(positions, dir) & positions;
  }
  template <size_t N>
  bool test_move(const coord (&mino)[N], const coord &dir) const {
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
  static constexpr array<inv_board_t, N> kick_positions(const inv_board_t &start, const inv_board_t &end, const coord (&kick)[N]) {
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
  static bool test_kick(const inv_board_t &start, const inv_board_t &end, const coord (&kick)[N]) {
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
    if (!usable[0][start[1] * W + start[0]]) {
      return ret;
    }
    ret[0].set(start[1] * W + start[0]);
    bool need_visit[4] = {true, false, false, false};
    for (bool updated = true; updated;) {
      updated = false;
      for (int i = 0; i < 4; ++i) {
        if (!need_visit[i]) {
          continue;
        }
        need_visit[i] = false;
        for (auto &move: MOVES) {
          inv_board_t mask = usable[i] & ~ret[i];
          inv_board_t to = move_to_center<true, true>(ret[i], move) & mask;
          if (to.any()) {
            ret[i] |= to;
            need_visit[i] = true;
            updated = true;
          }
        }
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
    if (!usable[0][start[1] * W + start[0]]) {
      return ret;
    }
    ret[0].set(start[1] * W + start[0]);
    queue<tuple<int, int, int>> q;
    q.emplace(start[0], start[1], 0);
    while (!q.empty()) {
      const auto [x, y, i] = q.front();
      q.pop();
      for (auto &[dx, dy]: MOVES) {
        if (usable[i].get(x + dx, y + dy) && !ret[i].get(x + dx, y + dy)) {
          ret[i].set((y + dy) * W + x + dx);
          q.emplace(x + dx, y + dy, i);
        }
      }
      for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 5; ++k) {
          auto &[dx, dy] = block.kicks[i][j][k];
          if (kicks[i][j][k].get(x, y) && !ret[(i + j + 1) % 4].get(x + dx, y + dy)) {
            ret[(i + j + 1) % 4].set((y + dy) * W + x + dx);
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
            ret[i].set((y + dy) * W + x + dx);
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
                ret[target].set((y + dy) * W + x + dx);
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
#define difficult
int main() {
#ifdef difficult
  vector<string> b_str = {
    "          ",
    "          ",
    "          ",
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
  };
#else
  vector<string> b_str = {
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
    "          ",
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
  };
#endif
  for (auto &s : b_str) {
    for (auto &c : s) {
      if (c == 'X') {
        c = '1';
      } else {
        c = '0';
      }
    }
  }
  string res;
  for (auto &s : b_str) {
    res.append(s.rbegin(), s.rend());
  }
  board<10, 22> b{bitset<10 * 22>{res}};
  block B = {
    {
#ifdef difficult
      {{-1, 1}, {0, 1}, {0, 0}, {1, 0}},   // 0
      {{1, 1}, {1, 0}, {0, 0}, {0, -1}},   // R
      {{1, -1}, {0, -1}, {0, 0}, {-1, 0}}, // 2
      {{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}  // L
#else
      {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},   // 0
      {{0, 1}, {0, 0}, {0, -1}, {1, 0}},   // R
      {{1, 0}, {0, 0}, {-1, 0}, {0, -1}}, // 2
      {{0, -1}, {0, 0}, {0, 1}, {-1, 0}}  // L
#endif
    },
    {
      { // 0
        {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},  // -> R
        {{0, 1}, {0, 1}, {0, 1}, {0, 1}, {0, 1}},       // -> 2
        {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}}      // -> L
      },
      { // R
        {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},      // -> 2
        {{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}},       // -> L
        {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}}       // -> 0
      },
      { // 2
        {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},     // -> L
        {{0, -1}, {0, -1}, {0, -1}, {0, -1}, {0, -1}},  // -> 0
        {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}   // -> R
      },
      { // L
        {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},   // -> 0
        {{-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // -> R
        {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}    // -> 2
      }
    }
  };
  auto binary = b.binary_bfs(B, {4, 20});
  auto ordinary = b.ordinary_bfs(B, {4, 20});
  auto ordinary_without_binary = b.ordinary_bfs_without_binary(B, {4, 20});
  for (int i = 0; i < 4; ++i) {
    if (binary[i].data != ordinary[i].data) {
      cout << "binary[" << i << "] != ordinary[" << i << "]" << endl;
      cout << to_string(binary[i], ordinary[i], b.data) << endl;
    }
    if (binary[i].data != ordinary_without_binary[i].data) {
      cout << "binary[" << i << "] != ordinary_without_binary[" << i << "]" << endl;
      cout << to_string(binary[i], ordinary_without_binary[i], b.data) << endl;
    }
  }
  auto bench = [&](auto f) {
    auto start = chrono::system_clock::now();
    for (int _ = 0; _ < 100000; ++_) {
      f();
    }
    auto end = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
  };
  cout << "binary: " << bench([&](){b.binary_bfs(B, {4, 20});}) << endl;
  cout << "ordinary: " << bench([&](){b.ordinary_bfs(B, {4, 20});}) << endl;
  cout << "ordinary_without_binary: " << bench([&](){b.ordinary_bfs_without_binary(B, {4, 20});}) << endl;
}