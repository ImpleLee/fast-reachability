#pragma once
#include "block.hpp"
#include "utils.hpp"
#include <limits>
#include <string_view>
#include <string>
#include <bitset>
#include <array>
#include <type_traits>
#include <cstdint>
#include <bit>
#include <experimental/simd>
#include "immintrin.h"

namespace reachability {
  template <unsigned W, unsigned H>
  struct board_t {
    static constexpr int width = W;
    static constexpr int height = H;
    static constexpr int block_size = 8;
    static constexpr int blocks_per_row = (H - 1) / block_size + 1;
    using under_t = std::uint8_t;
    static constexpr int num_of_under = std::bit_ceil(W * blocks_per_row);
    constexpr board_t() {
    }
    constexpr board_t(std::string_view s): board_t(convert_to_array(s)) {
    }
    constexpr board_t(std::array<under_t, num_of_under> d): data{d.data(), std::experimental::element_aligned} {
    }
    static constexpr std::array<under_t, num_of_under> convert_to_array(std::string_view s) {
      std::string reversed = std::string(s.rbegin(), s.rend());
      std::array<under_t, num_of_under> data = {0};
      for (std::size_t y = 0; y < H; ++y) {
        for (std::size_t x = 0; x < W; ++x) {
          if (reversed[y * W + x] == 'X') {
            data[y / block_size * W + x] |= under_t(1) << (y % block_size);
          }
        }
      }
      return data;
    }
    template <int x, int y>
    constexpr void set() {
      data[y / block_size * W + x] |= under_t(1) << (y % block_size);
    }
    template <int x, int y>
    constexpr int get() const {
      if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) {
        return 2;
      }
      return (data[y / block_size * W + x] >> (y % block_size)) & 1;
    }
    constexpr bool any() const {
      return *this != board_t{};
    }
    constexpr bool operator!=(const board_t &other) const {
      return any_of(data != other.data);
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
      if constexpr (dx != 0) {
        auto original = data;
        data = data_t([original]<std::size_t i>(std::integral_constant<std::size_t, i>) {
          constexpr auto column = i / W, row = i % W;
          if constexpr (column >= blocks_per_row) {
            return under_t(0);
          } else if constexpr (dx > 0 && row < dx) {
            return under_t(0);
          } else if constexpr (dx < 0 && row >= W + dx) {
            return under_t(0);
          } else {
            return original[i - dx];
          }
        });
      }
      if constexpr (dy < 0) {
        auto not_moved = data >> -dy;
        auto moved = data_t([moved = data << (block_size + dy)]<std::size_t i>(std::integral_constant<std::size_t, i>) {
          if constexpr (i + W < W * blocks_per_row) {
            return moved[i + W];
          } else {
            return under_t(0);
          }
        });
        data = not_moved | moved;
      } else if constexpr (dy > 0) {
        auto not_moved = data << dy;
        auto moved = data_t([moved = data >> (block_size - dy)]<std::size_t i>(std::integral_constant<std::size_t, i>) {
          if constexpr (i >= W && i < W * blocks_per_row) {
            return moved[i - W];
          } else {
            return under_t(0);
          }
        });
        data = not_moved | moved;
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
    alignas(std::experimental::memory_alignment_v<data_t>) data_t data = 0;
    template <std::size_t N>
    static constexpr std::experimental::fixed_size_simd<under_t, N> zero = {};
    static constexpr data_t mask_board() {
      data_t data = {0};
      data = ~data;
      static_assert(H % block_size == 0);
      for (std::size_t i = H / block_size * W; i < num_of_under; ++i) {
        data[i] = 0;
      }
      return data;
    }
  };
}

template <>
struct reachability::board_t<10, 24> {
  static constexpr int W = 10, H = 24;
  static constexpr int block_size = 8;
  static constexpr int blocks_per_row = 3;
  using under_t = std::uint8_t;
  static constexpr int num_of_under = 32;
  constexpr board_t() {
  }
  constexpr board_t(std::string_view s): board_t(convert_to_array(s)) {
  }
  constexpr board_t(std::array<under_t, num_of_under> d): data{
    d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9],
    d[10], d[11], d[12], d[13], d[14], d[15], d[16], d[17], d[18], d[19],
    d[20], d[21], d[22], d[23], d[24], d[25], d[26], d[27], d[28], d[29],
    0, 0
  } {
  }
  static constexpr std::array<under_t, num_of_under> convert_to_array(std::string_view s) {
    std::string reversed = std::string(s.rbegin(), s.rend());
    std::array<under_t, num_of_under> data = {0};
    for (std::size_t y = 0; y < H; ++y) {
      for (std::size_t x = 0; x < W; ++x) {
        if (reversed[y * W + x] == 'X') {
          data[y / block_size * W + x] |= under_t(1) << (y % block_size);
        }
      }
    }
    return data;
  }
  template <int x, int y>
  constexpr void set() {
    data[y / block_size * W + x] |= under_t(1) << (y % block_size);
  }
  template <int x, int y>
  constexpr int get() const {
    if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) {
      return 2;
    }
    return (data[y / block_size * W + x] >> (y % block_size)) & 1;
  }
  constexpr bool any() const {
    return *this != board_t{};
  }
  constexpr bool operator!=(const board_t &other) const {
    return _mm256_movemask_epi8(data != other.data);
  }
  constexpr board_t operator~() const {
    board_t other;
    other.data = mask_board & ~data;
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
    if constexpr (dx != 0) {
      data = permute<[](std::size_t i) -> std::size_t {
        auto column = i / W, row = i % W;
        if (column >= blocks_per_row) {
          return num_of_under;
        } else if (dx > 0 && row < dx) {
          return num_of_under;
        } else if (dx < 0 && row >= W + dx) {
          return num_of_under;
        } else {
          return i - dx;
        }
      }>(data);
    }
    if constexpr (dy < 0) {
      auto not_moved = data >> -dy;
      auto moved = permute<[](std::size_t i) -> std::size_t {
        if (i + W < W * blocks_per_row) {
          return i + W;
        } else {
          return num_of_under;
        }
      }>(data << (block_size + dy));
      data = not_moved | moved;
    } else if constexpr (dy > 0) {
      auto not_moved = data << dy;
      auto moved = permute<[](std::size_t i) -> std::size_t {
        if (i >= W && i < W * blocks_per_row) {
          return i - W;
        } else {
          return num_of_under;
        }
      }>(data >> (block_size - dy));
      data = not_moved | moved;
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
private:
  typedef std::uint8_t data_t [[gnu::vector_size(32)]];
  data_t data = {0};
  template <std::size_t N>
  static constexpr std::experimental::fixed_size_simd<under_t, N> zero = {};
  static constexpr data_t mask_board = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00
  };
  template <auto f>
  static constexpr data_t permute(data_t data) {
    return [=]<std::size_t... i>(std::index_sequence<i...>) {
      return __builtin_shufflevector(data, data_t{0}, f(i)...);
    }(std::make_index_sequence<32>());
  }
};