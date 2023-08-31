#pragma once
#include "block.hpp"
#include "utils.hpp"
#include <limits>
#include <string>
#include <bitset>
#include <array>
#include <type_traits>
#include <cstdint>
#include <bit>
#include <experimental/simd>

namespace reachability {
  template <unsigned W, unsigned H, typename under_t=std::uint64_t>
    requires std::numeric_limits<under_t>::is_integer && std::is_unsigned_v<under_t>
  struct board_t {
    static constexpr int under_bits = std::numeric_limits<under_t>::digits;
    static_assert(under_bits >= W);
    static constexpr int width = W;
    static constexpr int height = H;
    static constexpr int lines_per_under = under_bits / W;
    static constexpr int used_bits_per_under = lines_per_under * W;
    static constexpr int num_of_under = (H - 1) / lines_per_under + 1;
    static constexpr int last = num_of_under - 1;
    static constexpr int remaining_per_under = under_bits - used_bits_per_under;
    static constexpr under_t mask = (~under_t(0)) >> remaining_per_under;
    static constexpr int remaining_in_last = num_of_under * used_bits_per_under - H * W;
    static constexpr under_t last_mask = mask >> remaining_in_last;
    constexpr board_t() { }
    constexpr board_t(const std::string &s) {
      for (std::size_t i = 0; i < last; ++i) {
        data[i] = static_cast<under_t>(std::bitset<used_bits_per_under>{s, W * H - (i + 1) * used_bits_per_under, used_bits_per_under, ' ', 'X'}.to_ullong());
      }
      data[last] = static_cast<under_t>(std::bitset<used_bits_per_under>{s, 0, used_bits_per_under - remaining_in_last, ' ', 'X'}.to_ullong());
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
    constexpr bool any() const {
      return reduce(data, std::bit_or<>{});
    }
    constexpr bool operator!=(const board_t &other) const {
      return (*this ^ other).any();
    }
    constexpr board_t operator~() const {
      board_t other;
      other.data = mask_board() & ~data;
      return other;
    }
    constexpr board_t &operator&=(const board_t &rhs) {
      data &= rhs.data;
      return *this;
    }
    constexpr board_t operator&(const board_t &rhs) const {
      board_t result = *this;
      result &= rhs;
      return result;
    }
    constexpr board_t &operator|=(const board_t &rhs) {
      data |= rhs.data;
      return *this;
    }
    constexpr board_t operator|(const board_t &rhs) const {
      board_t result = *this;
      result |= rhs;
      return result;
    }
    constexpr board_t &operator^=(const board_t &rhs) {
      data ^= rhs.data;
      return *this;
    }
    constexpr board_t operator^(const board_t &rhs) const {
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
    friend constexpr std::string to_string(const board_t &board) {
      std::string ret;
      static_for<H>([&](auto y) {
        std::string this_ret;
        static_for<W>([&](auto x) {
          this_ret += board.template get<x, y>() ? "[]" : "  ";
        });
        this_ret += '\n';
        ret = this_ret + ret;
      });
      return ret;
    }
    friend constexpr std::string to_string(const board_t &board1, const board_t &board2) {
      std::string ret;
      static_for<H>([&](auto y) {
        std::string this_ret;
        static_for<W>([&](auto x) {
          bool b1 = board1.template get<x, y>();
          bool b2 = board2.template get<x, y>();
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
    friend constexpr std::string to_string(const board_t &board1, const board_t &board2, const board_t &board_3) {
      std::string ret;
      static_for<H>([&](auto y) {
        std::string this_ret;
        static_for<W>([&](auto x) {
          bool tested[2] = {bool(board1.template get<x, y>()), bool(board2.template get<x, y>())};
          bool b3 = board_3.template get<x, y>();
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
      static_for<needed>([&](auto i) {
        auto temp = board;
        temp.template right_shift<1 << i>();
        board &= temp;
      });
      auto temp = board;
      temp.template right_shift<W - (1 << needed)>();
      board &= temp;
      int lines = 0;
      const auto remove_range = [](const board_t &data, unsigned start, unsigned end) {
        board_t below = data << (H * W - start);
        below >>= (H * W - start);
        board_t above = data >> end;
        above <<= start;
        return below | above;
      };
      static_for<H>([&](auto y){
        if (board.template get<0, y>()) {
          data = remove_range(data, (y - lines) * W, (y - lines + 1) * W);
          ++lines;
        }
      });
      return lines;
    }
  private:
    using data_t = std::experimental::fixed_size_simd<under_t, num_of_under>;
    alignas(std::experimental::memory_alignment_v<data_t>) data_t data = {};
    template <std::size_t N>
    static constexpr std::experimental::fixed_size_simd<under_t, N> zero = {};
    template <int dx>
    static constexpr data_t mask_move() {
      board_t mask;
      if constexpr (dx > 0) {
        static_for<dx>([&](auto i) {
          static_for<H>([&](auto j) {
            mask.template set<i, j>();
          });
        });
      } else if constexpr (dx < 0) {
        static_for<-dx>([&](auto i) {
          static_for<H>([&](auto j) {
            mask.template set<W - 1 - i, j>();
          });
        });
      }
      return (~mask).data;
    }
    static constexpr data_t mask_board() {
      board_t ret;
      static_for<last>([&](auto i) {
        ret.data[i] = mask;
      });
      ret.data[last] = last_mask;
      return ret.data;
    }
    template <int removed, bool from_right>
    static constexpr data_t my_split(data_t data) {
      if constexpr (removed == 0) {
        return data;
      } else if constexpr (from_right) {
        auto [carry, _] = split<num_of_under-removed, removed>(data);
        return to_fixed_size(concat(zero<removed>, carry));
      } else {
        auto [_, carry] = split<removed, num_of_under-removed>(data);
        return to_fixed_size(concat(carry, zero<removed>));
      }
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
  };
}