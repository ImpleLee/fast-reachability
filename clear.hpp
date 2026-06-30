#include "utils.hpp"
#include "block.hpp"
#include <array>
#include <tuple>

namespace reachability::clear {
  template <int n, typename board_t> requires (n <= board_t::width)
  constexpr board_t has_single_empty_span(board_t board) {
    if constexpr (n <= 0) return board_t{};
    const auto reversed = ~board;
    const auto has_single_span = reversed.get_heads().has_single_bit();
    const auto exact_n_width = (reversed & reversed.template move<coord{1-n, 0}>()).has_single_bit();
    return (has_single_span & exact_n_width).populate_highest_bit();
  }

  template <Wrap<mino_p> auto mino>
  constexpr auto width_counts() {
    constexpr auto ranges = blocks::mino_range<mino>();
    constexpr int min_y = ranges[1], max_y = ranges[3];
    std::array<int, max_y - min_y + 1> counts{};
    static_for<std::tuple_size_v<decltype(mino)>>([&](auto i) {
      counts[mino[i][1_szc] - min_y]++;
    });
    return counts;
  }
}
