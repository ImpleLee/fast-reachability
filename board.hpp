#pragma once
#include "block.hpp"
#include <string>
#include <bitset>
#include <array>
#include <tuple>
#include <type_traits>
#include <queue>

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

  template <int W, int H>
  struct board {
    struct inv_board_t;
    struct board_t;
    static constexpr std::size_t W2 = (64 / W) * W;
    static constexpr std::size_t H2 = (H - 1) / (64 / W) + 1;
    static constexpr std::size_t DIFF = H2 * W2 - H * W;
    #define DEF_DUAL_OPERATOR(ret, op, param, expr2) \
      constexpr ret operator op(param) const { \
        ret result = *this; \
        result op##= expr2; \
        return result; \
      }
    #define DEF_OPERATOR(ret, op, param, expr, expr2) \
      constexpr ret &operator op##=(param) { \
        for (std::size_t i = 0; i < H2; ++i) { \
          data[i] op##= expr; \
        } \
        return *this; \
      } \
      DEF_DUAL_OPERATOR(ret, op, param, expr2)
    #define DEF_BOARD(name, name2) \
      struct name { \
        friend struct name2; \
        constexpr name() { } \
        constexpr name(const std::string &s) { \
          for (std::size_t i = 0; i + 1 < H2; ++i) { \
            data[i] = std::bitset<W2>{s, W * H - (i + 1) * W2, W2, ' ', 'X'}; \
          } \
          data[H2 - 1] = std::bitset<W2>{s, 0, W2 - DIFF, ' ', 'X'}; \
        } \
        constexpr explicit name(const name2 &other) { \
          for (std::size_t i = 0; i < H2; ++i) \
            data[i] = ~other.data[i]; \
        } \
        constexpr name operator~() const { \
          name other; \
          for (std::size_t i = 0; i < H2; ++i) { \
            other.data[i] = ~data[i]; \
          } \
          return other; \
        } \
        constexpr void set(int x, int y) { \
          data[y / (64 / W)].set((y % (64 / W)) * W + x); \
        } \
        constexpr int get(int x, int y) const { \
          if ((x < 0) || (x >= W) || (y < 0) || (y >= H)) { \
            return 2; \
          } \
          return data[y / (64 / W)][(y % (64 / W)) * W + x]; \
        } \
        constexpr bool any() const { \
          for (std::size_t i = 0; i + 1 < H2; ++i) { \
            if (data[i].any()) { \
              return true; \
            } \
          } \
          if (((data[H2 - 1] << DIFF) >> DIFF).any()) { \
            return true; \
          } \
          return false; \
        } \
        constexpr bool operator!=(const name &other) const { \
          for (std::size_t i = 0; i + 1 < H2; ++i) { \
            if (data[i] != other.data[i]) { \
              return true; \
            } \
          } \
          if (((data[H2 - 1] << DIFF) >> DIFF) != ((other.data[H2 - 1] << DIFF) >> DIFF)) { \
            return true; \
          } \
          return false; \
        } \
        DEF_OPERATOR(name, &, const name &rhs, rhs.data[i], rhs) \
        DEF_OPERATOR(name, |, const name &rhs, rhs.data[i], rhs) \
        constexpr name & operator>>=(std::size_t i) { \
          data[H2 - 1] <<= DIFF; \
          data[H2 - 1] >>= DIFF; \
          for (std::size_t j = 0; j < H2; ++j) { \
            data[j] >>= i; \
            if (j + 1 < H2) { \
              data[j] |= data[j + 1] << (W2 - i); \
            } \
          } \
          return *this; \
        } \
        constexpr name & right_shift(std::size_t i) { \
          data[H2 - 1] <<= DIFF; \
          data[H2 - 1] >>= DIFF; \
          for (std::size_t j = 0; j < H2; ++j) { \
            data[j] >>= i; \
          } \
          return *this; \
        } \
        constexpr name & operator<<=(std::size_t i) { \
          for (std::size_t j = H2 - 1; j < H2; --j) { \
            data[j] <<= i; \
            if (j > 0) { \
              data[j] |= data[j - 1] >> (W2 - i); \
            } \
          } \
          return *this; \
        } \
        constexpr name & left_shift(std::size_t i) { \
          for (std::size_t j = H2 - 1; j < H2; --j) { \
            data[j] <<= i; \
          } \
          return *this; \
        } \
        DEF_DUAL_OPERATOR(name, >>, std::size_t j, j) \
        DEF_DUAL_OPERATOR(name, <<, std::size_t j, j) \
      private: \
        std::array<std::bitset<W2>, H2> data; \
      }
    DEF_BOARD(board_t, inv_board_t);
    DEF_BOARD(inv_board_t, board_t);
    #undef DEF_BOARD
    #undef DEF_OPERATOR
    #undef DEF_DUAL_OPERATOR
    friend constexpr std::string to_string(const board_t &board) {
      std::string ret;
      for (int y = H - 1; y >= 0; --y) {
        for (int x = 0; x < W; ++x) {
          ret += board.get(x, y) ? "[]" : "  ";
        }
        ret += '\n';
      }
      return ret;
    }
    friend constexpr std::string to_string(const inv_board_t &board1, const board_t &board2) {
      std::string ret;
      for (int y = H - 1; y >= 0; --y) {
        for (int x = 0; x < W; ++x) {
          bool b1 = board1.get(x, y);
          bool b2 = board2.get(x, y);
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
    friend constexpr std::string to_string(const inv_board_t &board1, const inv_board_t &board2, const board_t &board_3) {
      std::string ret;
      for (int y = H - 1; y >= 0; --y) {
        for (int x = 0; x < W; ++x) {
          bool tested[2] = {bool(board1.get(x, y)), bool(board2.get(x, y))};
          bool b3 = board_3.get(x, y);
          std::string symbols = "  <>[]%%";
          for (int i = 0; i < 2; ++i) {
            ret += symbols[b3 * 4 + tested[i] * 2 + i];
          }
        }
        ret += '\n';
      }
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
      int i = 1;
      for (; (W >> 1) >= i; i <<= 1) {
        board &= board >> i;
      }
      board &= board >> (W - i);
      int lines = 0;
      for (int y = 0; y < H; ++y) {
        if (board.get(0, y)) {
          data = remove_range(data, (y - lines) * W, (y - lines + 1) * W);
          ++lines;
        }
      }
      return lines;
    }
    #if __cpp_lib_constexpr_bitset
    #define CONSTEXPR_BITSET constexpr
    #else
    #define CONSTEXPR_BITSET const
    #endif
    template <typename my_board_t, int dx>
    CONSTEXPR_BITSET static inline my_board_t MASK = []() constexpr {
      my_board_t mask;
      if (dx < 0) {
        for (int i = 0; i < -dx; ++i) {
          for (int j = 0; j < H; ++j) {
            mask.set(i, j);
          }
        }
      } else if (dx > 0) {
        for (int i = 0; i < dx; ++i) {
          for (int j = 0; j < H; ++j) {
            mask.set(W - 1 - i, j);
          }
        }
      }
      return ~mask;
    }();
    #undef CONSTEXPR_BITSET
    template <coord d, bool reverse = false, bool check = true, class board_t>
    static constexpr board_t move_to_center(board_t board) {
      constexpr auto dx = reverse ? -d[0] : d[0];
      constexpr auto dy = reverse ? -d[1] : d[1];
      int move = dy * W + dx;
      if (dy == 0) {
        if (move < 0) {
          board.left_shift(-move);
        } else {
          board.right_shift(move);
        }
      } else {
        if (move < 0) {
          board <<= -move;
        } else {
          board >>= move;
        }
      }
      if (check && dx != 0) {
        board &= MASK<board_t, dx>;
      }
      return board;
    }
    template <std::array mino>
    constexpr inv_board_t usable_positions() const {
      inv_board_t positions = ~inv_board_t();
      static_for<std::tuple_size_v<decltype(mino)>>([&](auto i) {
        positions &= move_to_center<mino[i]>(inv_board_t{data});
      });
      return positions;
    }
    static constexpr inv_board_t landable_positions(const inv_board_t &usable) {
      return usable & ~(usable << W);
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
      for (std::size_t i = 1; i < N; ++i) {
        positions[i] &= ~temp;
        temp |= positions[i];
      }
      return positions;
    }
    template <unsigned KICKS, unsigned ROTATIONS>
    struct info {
      inv_board_t usable;
      inv_board_t ret;
      std::array<inv_board_t, KICKS> kicks[ROTATIONS];
    };
    template <blocks::block block, coord start, bool use_optimize=false>
    constexpr std::array<inv_board_t, block.ORIENTATIONS> binary_bfs() const {
      constexpr int orientations = block.ORIENTATIONS;
      constexpr int rotations = block.ROTATIONS;
      constexpr int kicks = block.KICK_PER_ROTATION;
      info<kicks, rotations> info[orientations];
      static_for<orientations>([&](auto i) {
        info[i].usable = usable_positions<block.minos[i]>();
      });
      static_for<orientations>([&](auto i) {
        static_for<rotations>([&](auto j) {
          constexpr auto target = block.rotation_target(i, j);
          info[i].kicks[j] = kick_positions<block.kicks[i][j]>(info[i].usable, info[target].usable);
        });
      });
      constexpr std::array<coord, 3> MOVES = {{{-1, 0}, {1, 0}, {0, -1}}};
      if (!info[0].usable.get(start[0], start[1])) {
        return {};
      }
      bool need_visit[orientations] = { true };
      std::array<inv_board_t, orientations> cache;
      cache[0].set(start[0], start[1]);
      for (bool updated = true; updated;) {
        updated = false;
        static_for<orientations>([&](auto i){
          if (!need_visit[i]) {
            return;
          }
          need_visit[i] = false;
          info[i].ret = cache[i];
          for (bool updated2 = true; updated2;) {
            updated2 = false;
            static_for<MOVES.size()>([&](auto j){
              constexpr auto move = MOVES[j];
              inv_board_t mask = info[i].usable & ~info[i].ret;
              if (!mask.any()) {
                return;
              }
              inv_board_t to = move_to_center<move, true>(info[i].ret);
              to &= mask;
              if (to.any()) {
                info[i].ret |= to;
                updated2 = true;
              }
            });
          }
          cache[i] = info[i].ret;
          static_for<rotations>([&](auto j){
            constexpr int target = block.rotation_target(i, j);
            inv_board_t to;
            static_for<kicks>([&](auto k){
              inv_board_t from = info[i].ret & info[i].kicks[j][k];
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
      for (int i = 0; i < orientations; ++i) {
        cache[i] &= landable_positions(info[i].usable);
      }
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
      bool visited[orientations][H][W] = {};
      std::array<inv_board_t, orientations> ret;
      if (!usable(0, start[0], start[1])) {
        return ret;
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
              ret[i].set(x + dx, y + dy);
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
                  ret[target].set(x + dx, y + dy);
              }
              break;
            }
          }
        }
      }
      return ret;
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