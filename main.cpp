#include <bitset>
#include <array>
using namespace std;

using coord = signed char[2];
using mino = coord[4];
using kick = coord[5];

struct block {
  mino minos[4];
  kick kicks[4][3];
};

template <unsigned N>
constexpr void remove_range(const bitset<N> &data, unsigned start, unsigned end) {
  bitset<N> below = data << (N - start);
  below >>= (N - start);
  bitset<N> above = data >> end;
  above <<= start;
  return below | above;
}

template <unsigned W, unsigned H>
struct board {
  using board_t = bitset<W * H>;
  board_t data;
  constexpr board() {}
  constexpr int clear_full_lines() {
    board_t board = data;
    int i = 1;
    for (; (W >> 1) >= i; i >>= 1) {
      board &= board >> i;
    }
    board &= board >> (W - i);
    int lines = 0;
    for (int y = 0; y < H; ++y) {
      if (board[y * W]) {
        data = remove_range(data, (y - lines) * W, (y - lines + 1) * W);
        ++lines;
      }
    }
    return lines;
  }
  static constexpr board_t relative_move(board_t temp, coord d) {
    constexpr const board_t L = [](){
      board_t data;
      for (int i = 0; i < H; ++i) {
        data.set(i * W);
      }
      return ~data;
    }();
    constexpr const board_t R = [](){
      board_t data;
      for (int i = 0; i < H; ++i) {
        data.set(i * W + W - 1);
      }
      return ~data;
    }();
    auto dx = d[0], dy = d[1];
    if (dy < 0) {
      temp <<= (-dy) * W;
    } else if (dy > 0) {
      temp >>= dy * W;
    }
    if (dx < 0) {
      temp >>= -dx;
      for (int i = 0; i < -dx; ++i) {
        temp &= R >> i;
      }
    } else if (dx > 0) {
      temp <<= dx;
      for (int i = 0; i < dx; ++i) {
        temp &= L << i;
      }
    }
    return temp;
  }
  constexpr board_t usable_positions(const mino &mino) const {
    board_t positions = ~board_t();
    for (const auto &coord : mino) {
      positions &= relative_move(~data, coord);
    }
    return positions;
  }
  constexpr board_t landable_positions(const mino &mino) const {
    board_t positions = usable_positions(mino);
    board_t temp = board_t();
    for (const auto &coord : mino) {
      temp |= relative_move(~data, coord);
    }
    temp <<= W;
    positions &= ~temp;
    return positions;
  }
  static constexpr array<board_t, 5> kick_positions(const board_t &start, const board_t &end, const kick &kick) {
    array<board_t, 5> positions;
    for (int i = 0; i < 5; ++i) {
      positions[i] = relative_move(end, kick[i]) & start;
    }
    board_t temp = positions[0];
    for (int i = 1; i < 5; ++i) {
      positions[i] &= ~temp;
      temp |= positions[i];
    }
    return positions;
  }
};

int main() {
  board<10, 20> b;
}