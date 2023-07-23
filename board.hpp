#pragma once
#include "block.hpp"
#include <limits>
#include <string>
#include <bitset>
#include <array>
#include <sys/types.h>
#include <tuple>
#include <type_traits>
#include <queue>
#include <cstdint>
#include <numeric>
#include <bit>

namespace reachability {
  namespace {
    template<typename F, std::size_t... S>
    constexpr void static_for(F&& function, std::index_sequence<S...>) {
        int unpack[] = {0,
            (void(function(std::integral_constant<std::size_t, S>{})), 0)...
        };

        (void) unpack;
    }

    template<std::size_t iterations, typename F>
    constexpr void static_for(F&& function) {
        static_for(std::forward<F>(function), std::make_index_sequence<iterations>());
    }
  }

  namespace {
    using under_t = std::uint64_t;
    static constexpr inline auto under_bits = std::numeric_limits<under_t>::digits;
    template <int i>
    static constexpr inline under_t one = [](){
      static_assert(i >= 0 && i < under_bits);
      return under_t(1) << i;
    }();
  }

  template <unsigned W, unsigned H>
  struct board {
    struct inv_board_t;
    struct board_t;
    static constexpr auto lines_per_under = under_bits / W;
    static constexpr auto used_per_under = lines_per_under * W;
    static constexpr auto num_of_under = (H - 1) / lines_per_under + 1;
    static constexpr auto last = num_of_under - 1;
    static constexpr auto remaining_per_under = under_bits - used_per_under;
    static constexpr auto mask = (~under_t(0)) >> remaining_per_under;
    static constexpr auto remaining_in_last = num_of_under * used_per_under - H * W;
    static constexpr auto last_mask = mask >> remaining_in_last;
    static constexpr std::array<unsigned, 2> convert(int x, int y) {
      return {(y % lines_per_under) * W + x, y / lines_per_under};
    }
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
    #define DEF_BOARD(name, name2) \
      struct name { \
        friend struct name2; \
        constexpr name() { } \
        constexpr name(const std::string &s) { \
          for (std::size_t i = 0; i < last; ++i) { \
            data[i] = std::bitset<used_per_under>{s, W * H - (i + 1) * used_per_under, used_per_under, ' ', 'X'}.to_ullong(); \
          } \
          data[last] = std::bitset<used_per_under>{s, 0, used_per_under - remaining_in_last, ' ', 'X'}.to_ullong(); \
        } \
        constexpr explicit name(const name2 &other) { \
          static_for<num_of_under>([&](auto i) { \
            data[i] = ~other.data[i]; \
          }); \
        } \
        constexpr name operator~() const { \
          name other; \
          static_for<num_of_under>([&](auto i) { \
            other.data[i] = ~data[i]; \
          }); \
          return other; \
        } \
        template <int x, int y> \
        constexpr void set() { \
          constexpr auto a = convert(x, y); \
          data[a[1]] |= one<a[0]>; \
        } \
        template <int x, int y> \
        constexpr int get() const { \
          if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) { \
            return 2; \
          } \
          constexpr auto a = convert(x, y); \
          return data[a[1]] & one<a[0]> ? 1 : 0; \
        } \
        constexpr bool any() const { \
          under_t ret = 0; \
          static_for<last>([&](auto i) { \
            ret |= data[i]; \
          }); \
          ret &= mask; \
          ret |= data[last] & last_mask; \
          return ret; \
        } \
        constexpr bool operator!=(const name &other) const { \
          under_t ret = 0; \
          static_for<last>([&](auto i) { \
            ret |= data[i] ^ other.data[i]; \
          }); \
          ret &= mask; \
          ret |= (data[last] ^ other.data[last]) & last_mask; \
          return ret; \
        } \
        DEF_OPERATOR(name, &, const name &rhs, rhs.data[i], rhs) \
        DEF_OPERATOR(name, |, const name &rhs, rhs.data[i], rhs) \
        constexpr name & operator>>=(std::size_t i) { \
          static_for<last>([&](auto j) { \
            data[j] &= mask; \
            data[j] >>= i; \
            data[j] |= data[j + 1] << (used_per_under - i); \
          }); \
          data[last] &= last_mask; \
          data[last] >>= i; \
          return *this; \
        } \
        template <std::size_t i> \
        constexpr name & right_shift_carry() { \
          static_for<last>([&](auto j) { \
            data[j] &= mask; \
            data[j] >>= i; \
            data[j] |= data[j + 1] << (used_per_under - i); \
          }); \
          data[last] &= last_mask; \
          data[last] >>= i; \
          return *this; \
        } \
        template <std::size_t i> \
        constexpr name & right_shift() { \
          static_for<last>([&](auto j) { \
            data[j] &= mask; \
            data[j] >>= i; \
          }); \
          data[last] &= last_mask;\
          data[last] >>= i; \
          return *this; \
        } \
        constexpr name & operator<<=(std::size_t i) { \
          static_for<last>([&](auto j) { \
            data[last - j] <<= i; \
            data[last - j] |= (data[last - j - 1] & mask) >> (used_per_under - i); \
          }); \
          data[0] <<= i; \
          return *this; \
        } \
        template <std::size_t i> \
        constexpr name & left_shift_carry() { \
          static_for<last>([&](auto j) { \
            data[last - j] <<= i; \
            data[last - j] |= (data[last - j - 1] & mask) >> (used_per_under - i); \
          }); \
          data[0] <<= i; \
          return *this; \
        } \
        template <std::size_t i> \
        constexpr name & left_shift() { \
          static_for<num_of_under>([&](auto j) { \
            data[j] <<= i; \
          }); \
          return *this; \
        } \
        DEF_DUAL_OPERATOR(name, >>, std::size_t j, j) \
        DEF_DUAL_OPERATOR(name, <<, std::size_t j, j) \
      private: \
        std::array<under_t, num_of_under> data = {}; \
      }
    DEF_BOARD(board_t, inv_board_t);
    DEF_BOARD(inv_board_t, board_t);
    #undef DEF_BOARD
    #undef DEF_OPERATOR
    #undef DEF_DUAL_OPERATOR
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
    friend constexpr std::string to_string(const inv_board_t &board) {
      std::string ret;
      static_for<H>([&](auto y) {
        std::string this_ret;
        static_for<W>([&](auto x) {
          this_ret += board.template get<x, y>() ? "<>" : "  ";
        });
        this_ret += '\n';
        ret = this_ret + ret;
      });
      return ret;
    }
    friend constexpr std::string to_string(const inv_board_t &board1, const board_t &board2) {
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
    friend constexpr std::string to_string(const inv_board_t &board1, const inv_board_t &board2, const board_t &board_3) {
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
    template <typename my_board_t, int dx>
    static constexpr my_board_t MASK = []() consteval {
      my_board_t mask;
      if constexpr (dx < 0) {
        static_for<-dx>([&](auto i) {
          static_for<H>([&](auto j) {
            mask.template set<i, j>();
          });
        });
      } else if constexpr (dx > 0) {
        static_for<dx>([&](auto i) {
          static_for<H>([&](auto j) {
            mask.template set<W - 1 - i, j>();
          });
        });
      }
      return ~mask;
    }();
    template <coord d, bool reverse = false, bool check = true, class board_t>
    static constexpr board_t move_to_center(board_t board) {
      constexpr auto dx = reverse ? -d[0] : d[0];
      constexpr auto dy = reverse ? -d[1] : d[1];
      constexpr int move = dy * W + dx;
      if (dy == 0) {
        if constexpr (move < 0) {
          board.template left_shift<-move>();
        } else {
          board.template right_shift<move>();
        }
      } else {
        if constexpr (move < 0) {
          board.template left_shift_carry<-move>();
        } else {
          board.template right_shift_carry<move>();
        }
      }
      if constexpr (check && dx != 0) {
        board &= MASK<board_t, dx>;
      }
      return board;
    }
    template <std::array mino>
    constexpr inv_board_t usable_positions() const {
      inv_board_t positions = ~inv_board_t();
      inv_board_t temp = inv_board_t{data};
      static_for<std::tuple_size_v<decltype(mino)>>([&](auto i) {
        positions &= move_to_center<mino[i]>(temp);
      });
      return positions;
    }
    static constexpr inv_board_t landable_positions(const inv_board_t &usable) {
      auto temp = usable;
      temp.template left_shift_carry<W>();
      return usable & ~temp;
    }
    template <std::array kick>
    static constexpr std::array<inv_board_t, std::tuple_size_v<decltype(kick)>> kick_positions(const inv_board_t &start, const inv_board_t &end) {
      constexpr std::size_t N = std::tuple_size_v<decltype(kick)>;
      std::array<inv_board_t, N> positions;
      static_for<N>([&](auto i) {
        positions[i] = move_to_center<kick[i]>(end);
        positions[i] &= start;
      });
      inv_board_t temp = positions[0];
      static_for<N-1>([&](auto i) {
        positions[i + 1] &= ~temp;
        temp |= positions[i + 1];
      });
      return positions;
    }
    template <blocks::block block, coord start, bool use_optimize=false>
    constexpr std::array<inv_board_t, block.ORIENTATIONS> binary_bfs() const {
      constexpr int orientations = block.ORIENTATIONS;
      constexpr int rotations = block.ROTATIONS;
      constexpr int kicks = block.KICK_PER_ROTATION;
      inv_board_t usable[orientations];
      std::array<inv_board_t, kicks> kicks2[orientations][rotations];
      static_for<orientations>([&](auto i) {
        usable[i] = usable_positions<block.minos[i]>();
      });
      static_for<orientations>([&](auto i) {
        static_for<rotations>([&](auto j) {
          constexpr auto target = block.rotation_target(i, j);
          kicks2[i][j] = kick_positions<block.kicks[i][j]>(usable[i], usable[target]);
        });
      });
      constexpr std::array<coord, 3> MOVES = {{{-1, 0}, {1, 0}, {0, -1}}};
      if (!usable[0].template get<start[0], start[1]>()) {
        return {};
      }
      bool need_visit[orientations] = { true };
      std::array<inv_board_t, orientations> cache;
      cache[0].template set<start[0], start[1]>();
      for (bool updated = true; updated;) {
        updated = false;
        static_for<orientations>([&](auto i){
          if (!need_visit[i]) {
            return;
          }
          need_visit[i] = false;
          for (bool updated2 = true; updated2;) {
            updated2 = false;
            static_for<MOVES.size()>([&](auto j){
              constexpr auto move = MOVES[j];
              inv_board_t mask = usable[i] & ~cache[i];
              if (!mask.any()) {
                return;
              }
              inv_board_t to = move_to_center<move, true>(cache[i]);
              to &= mask;
              if (to.any()) {
                cache[i] |= to;
                updated2 = true;
              }
            });
          }
          static_for<rotations>([&](auto j){
            constexpr int target = block.rotation_target(i, j);
            inv_board_t to;
            static_for<kicks>([&](auto k){
              inv_board_t from = cache[i] & kicks2[i][j][k];
              to |= move_to_center<block.kicks[i][j][k], true, false>(from);
            });
            inv_board_t det = to & ~cache[target];
            if (det.any()) {
              cache[target] |= to;
              need_visit[target] = true;
              if (target < i)
                updated = true;
            }
          });
        });
      }
      static_for<orientations>([&](auto i){
        cache[i] &= landable_positions(usable[i]);
      });
      return cache;
    }
    template <coord start, bool use_optimize=false>
    [[gnu::noinline]]
    constexpr void binary_bfs(inv_board_t *ret, char block) const {
      blocks::call_with_block(block, [&]<blocks::block B>() {
        auto info = binary_bfs<B, start, use_optimize>();
        for (std::size_t i = 0; i < info.size(); ++i) {
          ret[i] = info[i];
        }
      });
    }
    auto ordinary_bfs_without_binary(const auto &block, const coord &start) const {
      constexpr auto orientations = std::remove_cvref_t<decltype(block)>::ORIENTATIONS;
      bool my_data[H][W];
      static_for<H>([&](auto y) {
        static_for<W>([&](auto x) {
          my_data[y][x] = data.template get<x, y>();
        });
      });
      auto in_range = [](int x, int y) {
        return 0 <= x && unsigned(x) < W && 0 <= y && unsigned(y) < H;
      };
      auto usable = [&](int i, int x, int y) {
        for (const auto &[dx, dy] : block.minos[i]) {
          if (!in_range(x + dx, y + dy)) return false;
          if (my_data[y + dy][x + dx]) return false;
        }
        return true;
      };
      constexpr const coord MOVES[] = {{0, -1}, {-1, 0}, {1, 0}};
      bool visited[orientations][H][W] = {};
      bool ret[orientations][H][W] = {};
      if (!usable(0, start[0], start[1])) {
        return std::array<inv_board_t, orientations>{};
      }
      visited[0][start[1]][start[0]] = true;
      std::queue<std::tuple<int, int, int>> q;
      q.emplace(start[0], start[1], 0);
      while (!q.empty()) {
        const auto [x, y, i] = q.front();
        q.pop();
        for (auto &[dx, dy] : MOVES) {
          if (in_range(x + dx, y + dy) && !visited[i][y + dy][x + dx] && usable(i, x + dx, y + dy)) {
            visited[i][y + dy][x + dx] = true;
            q.emplace(x + dx, y + dy, i);
            if (!usable(i, x + dx, y + dy - 1))
              ret[i][y + dy][x + dx] = true;
          }
        }
        for (int j = 0; j < block.ROTATIONS; ++j) {
          const auto target = block.rotation_target(i, j);
          for (auto &[dx, dy] : block.kicks[i][j]) {
            if (in_range(x + dx, y + dy) && usable(target, x + dx, y + dy)) {
              if (!visited[target][y + dy][x + dx]) {
                visited[target][y + dy][x + dx] = true;
                q.emplace(x + dx, y + dy, target);
                if (!usable(target, x + dx, y + dy - 1))
                  ret[target][y + dy][x + dx] = true;
              }
              break;
            }
          }
        }
      }
      std::array<inv_board_t, orientations> true_ret;
      static_for<orientations>([&](auto i) {
        static_for<H>([&](auto y) {
          static_for<W>([&](auto x) {
            if (ret[i][y][x]) {
              true_ret[i].template set<x, y>();
            }
          });
        });
      });
      return true_ret;
    }
    [[gnu::noinline]]
    constexpr void ordinary_bfs_without_binary(inv_board_t *ret, char block, const coord &start) const {
      blocks::call_with_block(block, [&]<blocks::block B>() {
        auto info = ordinary_bfs_without_binary(B, start);
        for (std::size_t i = 0; i < info.size(); ++i) {
          ret[i] = info[i];
        }
      });
    }
  };
}