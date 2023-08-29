#pragma once
#include <type_traits>
#include <utility>
namespace reachability {
  template<typename F, std::size_t... S>
  [[gnu::always_inline]]
  constexpr void static_for(F&& function, std::index_sequence<S...>) {
      int unpack[] = {0,
          (void(function(std::integral_constant<std::size_t, S>{})), 0)...
      };

      (void) unpack;
  }

  template<std::size_t iterations, typename F>
  [[gnu::always_inline]]
  constexpr void static_for(F&& function) {
      static_for(std::forward<F>(function), std::make_index_sequence<iterations>());
  }

  template <typename T, std::size_t N>
  struct static_vector {
    static constexpr std::size_t capacity = N;
    T data[N];
    std::size_t used = 0;
    constexpr std::size_t size() const {
      return used;
    }
    constexpr const T &operator[](std::size_t i) const {
      return data[i];
    }
    constexpr T &operator[](std::size_t i) {
      return data[i];
    }
    constexpr void push_back(const T &t) {
      data[used++] = t;
    }
  };
}