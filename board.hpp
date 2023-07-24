#pragma once
#include "utils.hpp"
#include <limits>
#include <string>
#include <bitset>
#include <array>
#include <type_traits>
#include <cstdint>
#include <bit>

namespace reachability::board {
  using namespace utils;
  using under_t = std::uint64_t;
  static constexpr inline auto under_bits = std::numeric_limits<under_t>::digits;
  template <int i>
  static constexpr inline under_t one = [](){
    static_assert(i >= 0 && i < under_bits);
    return under_t(1) << i;
  }();
  template <unsigned W, unsigned H> struct board_t;

  #define DEF_DUAL_OPERATOR(ret, op, param, expr2) \
    constexpr ret operator op(param) const { \
      ret result = *this; \
      result op##= expr2; \
      return result; \
    }
  #define DEF_OPERATOR(ret, op, param, expr, expr2) \
    constexpr ret &operator op##=(param) { \
      static_for<num_of_under>([&](auto i) { \
        data[i] op##= expr; \
      }); \
      return *this; \
    } \
    DEF_DUAL_OPERATOR(ret, op, param, expr2)

  template <unsigned W, unsigned H>
  struct board_t {
    static constexpr auto width = W;
    static constexpr auto height = H;
    static constexpr auto lines_per_under = under_bits / W;
    static constexpr auto used_per_under = lines_per_under * W;
    static constexpr auto num_of_under = (H - 1) / lines_per_under + 1;
    static constexpr auto last = num_of_under - 1;
    static constexpr auto remaining_per_under = under_bits - used_per_under;
    static constexpr auto mask = (~under_t(0)) >> remaining_per_under;
    static constexpr auto remaining_in_last = num_of_under * used_per_under - H * W;
    static constexpr auto last_mask = mask >> remaining_in_last;
    constexpr board_t() { }
    constexpr board_t(const std::string &s) {
      for (std::size_t i = 0; i < last; ++i) {
        data[i] = std::bitset<used_per_under>{s, W * H - (i + 1) * used_per_under, used_per_under, ' ', 'X'}.to_ullong();
      }
      data[last] = std::bitset<used_per_under>{s, 0, used_per_under - remaining_in_last, ' ', 'X'}.to_ullong();
    }
    template <int x, int y>
    constexpr void set() {
      data[y / lines_per_under] |= one<(y % lines_per_under) * W + x>;
    }
    template <int x, int y>
    constexpr int get() const {
      if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) {
        return 2;
      }
      return data[y / lines_per_under] & one<(y % lines_per_under) * W + x> ? 1 : 0;
    }
    constexpr bool any() const {
      under_t ret = 0;
      static_for<last>([&](auto i) {
        ret |= data[i];
      });
      ret &= mask;
      ret |= data[last] & last_mask;
      return ret;
    }
    constexpr bool operator!=(const board_t &other) const {
      return (*this ^ other).any();
    }
    constexpr board_t operator~() const {
      board_t other;
      static_for<num_of_under>([&](auto i) {
        other.data[i] = ~data[i];
      });
      return other;
    }
    constexpr board_t &operator&=(const board_t &rhs) {
      static_for<num_of_under>([&](auto i) {
        data[i] &= rhs.data[i];
      });
      return *this;
    }
    constexpr board_t operator&(const board_t &rhs) const {
      board_t result = *this;
      result &= rhs;
      return result;
    }
    constexpr board_t &operator|=(const board_t &rhs) {
      static_for<num_of_under>([&](auto i) {
        data[i] |= rhs.data[i];
      });
      return *this;
    }
    constexpr board_t operator|(const board_t &rhs) const {
      board_t result = *this;
      result |= rhs;
      return result;
    }
    constexpr board_t &operator^=(const board_t &rhs) {
      static_for<num_of_under>([&](auto i) {
        data[i] ^= rhs.data[i];
      });
      return *this;
    }
    constexpr board_t operator^(const board_t &rhs) const {
      board_t result = *this;
      result ^= rhs;
      return result;
    }
    constexpr board_t & operator>>=(std::size_t i) {
      static_for<last>([&](auto j) {
        data[j] &= mask;
        data[j] >>= i;
        data[j] |= data[j + 1] << (used_per_under - i);
      });
      data[last] &= last_mask;
      data[last] >>= i;
      return *this;
    }
    constexpr board_t operator>>(std::size_t i) const {
      board_t result = *this;
      result >>= i;
      return result;
    }
    template <std::size_t i>
    constexpr board_t & right_shift_carry() {
      static_for<last>([&](auto j) {
        data[j] &= mask;
        data[j] >>= i;
        data[j] |= data[j + 1] << (used_per_under - i);
      });
      data[last] &= last_mask;
      data[last] >>= i;
      return *this;
    }
    template <std::size_t i>
    constexpr board_t & right_shift() {
      static_for<last>([&](auto j) {
        data[j] &= mask;
        data[j] >>= i;
      });
      data[last] &= last_mask;\
      data[last] >>= i;
      return *this;
    }
    constexpr board_t & operator<<=(std::size_t i) {
      static_for<last>([&](auto j) {
        data[last - j] <<= i;
        data[last - j] |= (data[last - j - 1] & mask) >> (used_per_under - i);
      });
      data[0] <<= i;
      return *this;
    }
    constexpr board_t operator<<(std::size_t i) const {
      board_t result = *this;
      result <<= i;
      return result;
    }
    template <std::size_t i>
    constexpr board_t & left_shift_carry() {
      static_for<last>([&](auto j) {
        data[last - j] <<= i;
        data[last - j] |= (data[last - j - 1] & mask) >> (used_per_under - i);
      });
      data[0] <<= i;
      return *this;
    }
    template <std::size_t i>
    constexpr board_t & left_shift() {
      static_for<num_of_under>([&](auto j) {
        data[j] <<= i;
      });
      return *this;
    }
  private:
    std::array<under_t, num_of_under> data = {};
  };

  template <unsigned W, unsigned H>
  constexpr std::string to_string(const board_t<W, H> &board) {
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
  template <unsigned W, unsigned H>
  constexpr std::string to_string(const board_t<W, H> &board1, const board_t<W, H> &board2) {
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
  template <unsigned W, unsigned H>
  constexpr std::string to_string(const board_t<W, H> &board1, const board_t<W, H> &board2, const board_t<W, H> &board_3) {
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

  template <unsigned W, unsigned H>
  constexpr board_t<W, H> remove_range(const board_t<W, H> &data, unsigned start, unsigned end) {
    board_t<W, H> below = data << (H * W - start);
    below >>= (H * W - start);
    board_t<W, H> above = data >> end;
    above <<= start;
    return below | above;
  }

  template <unsigned W, unsigned H>
  constexpr int clear_full_lines(board_t<W, H> &data) {
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
    static_for<H>([&](auto y){
      if (board.template get<0, y>()) {
        data = remove_range(data, (y - lines) * W, (y - lines + 1) * W);
        ++lines;
      }
    });
    return lines;
  }
}
namespace reachability {
  using board::board_t;
}