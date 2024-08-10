#include "block.hpp"
#include "board.hpp"
#include "search.hpp"
#include <string_view>
#include <iostream>
#include "bench.hpp"
using namespace std;

bool _ = ios::sync_with_stdio(false);
template <reachability::block block, bool print=false, reachability::coord start=reachability::coord{4, 20}, unsigned init_rot=0>
array<double, 2> test(const BOARD &b, string_view name) {
  using namespace reachability::search;
  cout << "BOARD " << name << endl;
  auto binary = binary_bfs<block, start, init_rot>(b);
  auto ordinary = ordinary_bfs_without_binary(b, block, start, init_rot);
  if (binary.size() != ordinary.size()) {
    cout << "  binary.size() != ordinary.size()" << endl;
    cout << "  binary.size() = " << binary.size() << endl;
    cout << "  ordinary.size() = " << ordinary.size() << endl;
  }
  for (size_t i = 0; i < min(binary.size(), ordinary.size()); ++i) {
    if (binary[i] != ordinary[i]) {
      cout << "  binary[" << i << "] != ordinary[" << i << "]" << endl;
      cout << to_string(binary[i], ordinary[i], b);
    } else if (print) {
      cout << "  result[" << i << "]" << endl;
      cout << to_string(binary[i], b);
    }
  }
  auto binary_time = bench<1000000>([](auto b){ return binary_bfs<block, start, init_rot>(b); }, b);
  cout << "  binary  : " << binary_time << " cycles" << endl;
  auto ordinary_time = bench([](auto b){ return ordinary_bfs_without_binary(b, block, start, init_rot); }, b);
  cout << "  true ord: " << ordinary_time << " cycles" << endl;
  return {binary_time, ordinary_time};
}
template <bool print=false, reachability::coord start=reachability::coord{4, 20}, unsigned init_rot=0>
array<double, 2> test(const BOARD &b, string_view name, char block) {
  using namespace reachability::search;
  using namespace reachability::blocks;
  cout << "BOARD " << name << endl;
  cout << " BLOCK " << block << endl;
  auto binary = binary_bfs<SRS, start, init_rot>(b, block);
  auto ordinary = ordinary_bfs_without_binary<SRS>(b, block, start, init_rot);
  if (binary.size() != ordinary.size()) {
    cout << "  binary.size() != ordinary.size()" << endl;
    cout << "  binary.size() = " << binary.size() << endl;
    cout << "  ordinary.size() = " << ordinary.size() << endl;
  }
  for (size_t i = 0; i < min(binary.size(), ordinary.size()); ++i) {
    if (binary[i] != ordinary[i]) {
      cout << "  binary[" << i << "] != ordinary[" << i << "]" << endl;
      cout << to_string(binary[i], ordinary[i], b);
    } else if (print) {
      cout << "  result[" << i << "]" << endl;
      cout << to_string(binary[i], b);
    }
  }
  auto binary_time = bench<1000000>([](auto b, auto block){ return binary_bfs<SRS, start, init_rot>(b, block); }, b, block);
  cout << "  binary  : " << binary_time << " cycles" << endl;
  auto ordinary_time = bench([](auto b, auto block){ return ordinary_bfs_without_binary<SRS>(b, block, start, init_rot); }, b, block);
  cout << "  true ord: " << ordinary_time << " cycles" << endl;
  return {binary_time, ordinary_time};
}
int main() {
  double binary_sum = 0, ordinary_sum = 0;
  for (size_t i = 0; i < board_names.size(); ++i) {
    for (char block : "IJLOSTZ"sv) {
      auto [binary_time, ordinary_time] = test(boards[i], board_names[i], block);
      binary_sum += binary_time;
      ordinary_sum += ordinary_time;
    }
  }
  cout << "TOTAL binary  : " << binary_sum << " cycles" << endl;
  cout << "TOTAL true ord: " << ordinary_sum << " cycles" << endl;
  using namespace reachability::blocks;
  constexpr auto lzt = merge_str({
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
  });
  test<SRS::Z, true>(lzt, "LZT");
  constexpr auto farter1 = merge_str({
    "     X    ",
    "XXX X     ",
    "         X",
    " X X  XX  ",
    " X        ",
    "    X X  X",
    "X   XXX   ",
    "X XX      ",
    "       X  ",
    "    X     ",
    "  XX X X  ",
    "X     XX  ",
    "   X X    ",
    " X      X ",
    "    X     ",
    "XX     X X",
    "    X X   ",
    "X  X      ",
    "    XX    ",
    "  X       ",
    "       X  ",
    " XXX XX X ",
  });
  test<SRS::L, true, reachability::coord{1, 1}, 2>(farter1, "FARTER 1");
  constexpr auto farter2 = merge_str({
    "       X  ",
    "       X  ",
    "   X   XX ",
    "        X ",
    "        X ",
    "    X   XX",
    "         X",
    "         X",
    "     X   X",
    "          ",
    "          ",
    "      X   ",
    "          ",
    "          ",
    "       X  ",
    "          ",
    "          ",
    "          "
  });
  test<SRS::I, true, reachability::coord{7, 0}, 0>(farter2, "FARTER 2");
}