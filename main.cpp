#include "block.hpp"
#include "board.hpp"
#include <string>
#include <deque>
#include <iostream>
#include <chrono>
using namespace std;

bool _ = ios::sync_with_stdio(false);
using BOARD = reachability::board<10, 22>;
BOARD str_to_board(deque<string> &&b_str) {
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
  const deque<BOARD> boards = {
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
  const deque<string> board_names = {
    "TSPIN", "DT", "BAD", "LZT", "4T"
  };
  const string name = "IJLOSTZ";
  for (size_t i = 0; i < board_names.size(); ++i) {
    cout << "BOARD " << board_names[i] << endl;
    const auto &b = boards[i];
    for (int j = 0; j < 7; ++j) {
      cout << " BLOCK " << name[j] << endl;
      BOARD::inv_board_t binary[4], ordinary[4];
      int length = reachability::blocks::get_orientations(name[j]);
      b.binary_bfs<reachability::coord{4, 20}>(binary, name[j]);
      b.ordinary_bfs_without_binary(ordinary, name[j], {4, 20});
      for (int i = 0; i < length; ++i) {
        if (binary[i] != ordinary[i]) {
          cout << "  binary[" << i << "] != ordinary[" << i << "]" << endl;
          cout << to_string(binary[i], ordinary[i], b.data) << endl;
        }
      }
      cout << "  binary  : " << bench([&](){b.binary_bfs<reachability::coord{4, 20}>(binary, name[j]);}) << "ns" << endl;
      cout << "  true ord: " << bench([&](){b.ordinary_bfs_without_binary(ordinary, name[j], {4, 20});}) << "ns" << endl;
    }
  }
}