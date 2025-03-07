#pragma once
#include "block.hpp"
#include "utils.hpp"
#include <limits>
#include <string_view>
#include <array>
#include <type_traits>
#include <cstdint>
#include <bit>
#include <experimental/simd>

namespace reachability {
  template <unsigned W, unsigned H, typename under_t=std::uint64_t>
    requires
      std::numeric_limits<under_t>::is_integer
      && std::is_unsigned_v<under_t>
      && (std::numeric_limits<under_t>::digits >= W)
  struct board_t {
    static constexpr int under_bits = std::numeric_limits<under_t>::digits;
    static constexpr int width = W;
    static constexpr int height = H;
    static constexpr int lines_per_under = under_bits / W;
    static constexpr int used_bits_per_under = lines_per_under * W;
    static constexpr int num_of_under = (H - 1) / lines_per_under + 1;
    static constexpr int last = num_of_under - 1;
    static constexpr int remaining_per_under = under_bits - used_bits_per_under;
    static constexpr under_t mask = under_t(-1) >> remaining_per_under;
    static constexpr int remaining_in_last = num_of_under * used_bits_per_under - H * W;
    static constexpr under_t last_mask = mask >> remaining_in_last;
    constexpr board_t() = default;
    constexpr board_t(std::string_view s): board_t(convert_to_array(s)) {}
    constexpr board_t(std::array<under_t, num_of_under> d): data{d.data(), std::experimental::element_aligned} {}
    static constexpr std::array<under_t, num_of_under> convert_to_array(std::string_view s) {
      std::array<under_t, num_of_under> data = {};
      for (std::size_t i = 0; i < last; ++i) {
        data[i] = convert_to_under_t(s.substr(W * H - (i + 1) * used_bits_per_under, used_bits_per_under));
      }
      data[last] = convert_to_under_t(s.substr(0, used_bits_per_under - remaining_in_last));
      return data;
    }
    template <int x, int y>
    constexpr void set() {
      data[y / lines_per_under] |= under_t(1) << ((y % lines_per_under) * W + x);
    }
    template <int x, int y>
    constexpr int get() const {
      if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) {
        return 2;
      }
      return data[y / lines_per_under] & (under_t(1) << ((y % lines_per_under) * W + x)) ? 1 : 0;
    }
    template <int y>
    constexpr int get() const {
      // use highest bit as the result
      return get<W - 1, y>();
    }
    constexpr bool any() const {
      return *this != board_t{};
    }
    constexpr bool operator!=(board_t other) const {
      return any_of(data != other.data);
    }
    constexpr bool contains(board_t other) const {
      return all_of((other.data & ~data) == 0);
    }
    constexpr board_t operator~() const {
      board_t other;
      other.data = mask_board() & ~data;
      return other;
    }
    constexpr board_t &operator&=(board_t rhs) {
      data &= rhs.data;
      return *this;
    }
    constexpr board_t operator&(board_t rhs) const {
      board_t result = *this;
      result &= rhs;
      return result;
    }
    constexpr board_t &operator|=(board_t rhs) {
      data |= rhs.data;
      return *this;
    }
    constexpr board_t operator|(board_t rhs) const {
      board_t result = *this;
      result |= rhs;
      return result;
    }
    constexpr board_t &operator^=(board_t rhs) {
      data ^= rhs.data;
      return *this;
    }
    constexpr board_t operator^(board_t rhs) const {
      board_t result = *this;
      result ^= rhs;
      return result;
    }
    template <coord d, bool check = true>
    constexpr void move_() {
      constexpr int dx = d[0], dy = d[1];
      if constexpr (dy == 0) {
        if constexpr (dx > 0) {
          data <<= dx;
          data &= mask_board();
        } else if constexpr (dx < 0) {
          data >>= -dx;
        }
      } else if constexpr (dy > 0) {
        constexpr int pad = (dy - 1) / lines_per_under;
        constexpr int shift = (dy - 1) % lines_per_under + 1;
        auto not_moved = my_shift<dx, shift>(my_split<pad, true>(data));
        auto moved = my_shift<dx, shift-lines_per_under>(my_split<pad+1, true>(data));
        data = (not_moved | moved) & mask_board();
      } else {
        constexpr int pad = (-dy - 1) / lines_per_under;
        constexpr int shift = (-dy - 1) % lines_per_under + 1;
        auto not_moved = my_shift<dx, -shift>(my_split<pad, false>(data));
        auto moved = my_shift<dx, lines_per_under-shift>(my_split<pad+1, false>(data));
        data = (not_moved | moved) & mask_board();
      }
      if constexpr (check && dx != 0) {
        data &= mask_move<dx>();
      }
    }
    template <coord d, bool check = true>
    constexpr board_t move() const {
      board_t result = *this;
      result.move_<d, check>();
      return result;
    }
    friend constexpr std::string to_string(board_t board) {
      std::string ret;
      static_for<H>([&][[gnu::always_inline]](auto y) {
        std::string this_ret;
        static_for<W>([&][[gnu::always_inline]](auto x) {
          this_ret += board.get<x, y>() ? "[]" : "  ";
        });
        this_ret += '\n';
        ret = this_ret + ret;
      });
      return ret;
    }
    friend constexpr std::string to_string(board_t board1, board_t board2) {
      std::string ret;
      static_for<H>([&][[gnu::always_inline]](auto y) {
        std::string this_ret;
        static_for<W>([&][[gnu::always_inline]](auto x) {
          bool b1 = board1.get<x, y>();
          bool b2 = board2.get<x, y>();
          if (b1 && b2) {
            this_ret += "%%";
          } else if (b1) {
            this_ret += "..";
          } else if (b2) {
            this_ret += "[]";
          } else {
            this_ret += "  ";
          }
        });
        this_ret += '\n';
        ret = this_ret + ret;
      });
      return ret;
    }
    friend constexpr std::string to_string(board_t board1, board_t board2, board_t board_3) {
      std::string ret;
      static_for<H>([&][[gnu::always_inline]](auto y) {
        std::string this_ret;
        static_for<W>([&][[gnu::always_inline]](auto x) {
          bool tested[2] = {bool(board1.get<x, y>()), bool(board2.get<x, y>())};
          bool b3 = board_3.get<x, y>();
          std::string symbols = "  <>[]%%";
          for (int i = 0; i < 2; ++i) {
            this_ret += symbols[b3 * 4 + tested[i] * 2 + i];
          }
        });
        this_ret += '\n';
        ret = this_ret + ret;
      });
      return ret;
    }
    constexpr int clear_full_lines() {
      auto board = data;
      constexpr int needed = std::numeric_limits<decltype(W)>::digits - std::countl_zero(W) - 1;
      static_for<needed>([&][[gnu::always_inline]](auto i) {
        auto temp = board;
        temp.template right_shift<1 << i>();
        board &= temp;
      });
      auto temp = board;
      temp.template right_shift<W - (1 << needed)>();
      board &= temp;
      int lines = 0;
      const auto remove_range = [](board_t data, unsigned start, unsigned end) {
        board_t below = data << (H * W - start);
        below >>= (H * W - start);
        board_t above = data >> end;
        above <<= start;
        return below | above;
      };
      static_for<H>([&][[gnu::always_inline]](auto y){
        if (board.template get<0, y>()) {
          data = remove_range(data, (y - lines) * W, (y - lines + 1) * W);
          ++lines;
        }
      });
      return lines;
    }
    constexpr board_t has_single_bit() const {
      auto saturated = data | one_bit<W - 1>();
      saturated &= saturated - one_bit<0>();
      // if data has no 1 in not-highest bits: 0
      // if data has exactly one 1 in not-highest bits: 10...0
      // if data has more than one 1 in not-highest bits: 1...1...
      auto saturated2 = saturated | one_bit<W - 1>();
      saturated2 &= saturated2 - one_bit<0>();
      auto result = (saturated ^ data) & ~saturated2;
      return to_board(result);
    }
    constexpr board_t all_bits() const {
      auto low = data & ~one_bit<W - 1>();
      return to_board(data & (low + one_bit<0>()));
    }
    constexpr board_t any_bit() const {
      return ~to_board(~data).all_bits();
    }
    constexpr board_t no_bit() const {
      return ~any_bit();
    }
    constexpr board_t remove_ones_after_zero() const {
      auto board = data | ~mask_board();
      std::array<int, num_of_under> ones;
      static_for<num_of_under>([&][[gnu::always_inline]](auto i) {
        ones[i] = std::countl_one(under_t(board[i]));
      });
      bool found = false;
      #pragma unroll num_of_under
      for (int i = num_of_under - 1; i >= 0; --i) {
        if (found) {
          board[i] = 0;
        } else if (ones[i] < std::numeric_limits<under_t>::digits) {
          found = true;
          board[i] &= ~((~under_t(0)) >> ones[i]);
        }
      }
      return to_board(board & mask_board());
    }
    constexpr board_t populate_highest_bit() const {
      // result is in highest bit (0 or 1), other bits are 0
      // populate the result to all bits
      auto result = data & one_bit<W - 1>();
      auto pre_result = one_bit<W - 1>() - (result >> (W - 1));
      return to_board(pre_result ^ one_bit<W - 1>());
    }
    constexpr board_t get_heads() const {
      return (*this) & ~move<coord{-1, 0}>();
    }
    friend constexpr board_t can_expand(board_t current, board_t possible) {
      const auto starts = possible & current.template move<coord{-1, 0}>();
      const auto ends = possible & current.template move<coord{1, 0}>();
      const auto all_heads = possible.get_heads();
      const auto heads = starts.data | (ends.data + (possible & ~all_heads).data);
      return to_board(heads);
    }
  private:
    template <std::size_t N>
    using simd_of = std::experimental::simd<under_t, std::experimental::simd_abi::deduce_t<under_t, N>>;
    using data_t = simd_of<num_of_under>;
    alignas(std::experimental::memory_alignment_v<data_t>) data_t data = 0;
    template <std::size_t N>
    static constexpr simd_of<N> zero = {};
    static constexpr board_t to_board(data_t data) {
      board_t ret;
      ret.data = data;
      return ret;
    }
    template <int dx>
    static constexpr data_t mask_move() {
      board_t mask;
      if constexpr (dx > 0) {
        static_for<dx>([&][[gnu::always_inline]](auto i) {
          static_for<H>([&][[gnu::always_inline]](auto j) {
            mask.set<i, j>();
          });
        });
      } else if constexpr (dx < 0) {
        static_for<-dx>([&][[gnu::always_inline]](auto i) {
          static_for<H>([&][[gnu::always_inline]](auto j) {
            mask.set<W - 1 - i, j>();
          });
        });
      }
      return (~mask).data;
    }
    template <int dx>
    static constexpr data_t one_bit() {
      board_t ret;
      static_for<H>([&][[gnu::always_inline]](auto j) {
        ret.set<dx, j>();
      });
      return ret.data;
    };
    static constexpr data_t mask_board() {
      return data_t{[](auto i) {
        if constexpr (i == last) {
          return last_mask;
        } else {
          return mask;
        }
      }};
    }
    template <int removed, bool from_right>
    static constexpr data_t my_split(data_t data) {
      return data_t([=][[gnu::always_inline]](auto i) {
        constexpr size_t index = from_right ? i - removed : i + removed;
        if constexpr (index >= num_of_under) {
          return 0;
        } else {
          return data[index];
        }
      });
    }
    template <int x_shift, int y_shift>
    static constexpr data_t my_shift(data_t data) {
      if constexpr (y_shift == lines_per_under || y_shift == -lines_per_under) {
        data = data_t(0);
      } else if constexpr (y_shift < 0) {
        data >>= -y_shift * W;
      } else if constexpr (y_shift > 0) {
        data <<= y_shift * W;
      }
      if constexpr (x_shift > 0) {
        data <<= x_shift;
      } else if constexpr (x_shift < 0) {
        data >>= -x_shift;
      }
      return data;
    }
    static constexpr under_t convert_to_under_t(std::string_view in) {
      under_t res = 0;
      for (char c : in) {
        res *= 2;
        if (c == 'X')
          res += 1;
      }
      return res;
    }
  };
}