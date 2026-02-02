#pragma once
#include <type_traits>
#include <utility>
#include <span>
#include <algorithm>
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
    std::size_t used;
    template <std::size_t M>
    constexpr static_vector(std::span<T, M> arr): used(M) {
      static_assert(M <= N);
      std::copy(arr.begin(), arr.end(), data);
    }
    static_vector() = delete;
    constexpr std::size_t size() const {
      return used;
    }
    constexpr const T &operator[](std::size_t i) const {
      return data[i];
    }
    constexpr T &operator[](std::size_t i) {
      return data[i];
    }
  };

  // home-made tuple
  // based on https://stackoverflow.com/a/69194245, CC BY-SA 4.0
  // what we actually want is std::vector, but it's not constexpr yet
  // so we use std::tuple instead, but it's not a structural type sadly
  namespace details {
    template <std::size_t I, typename T> struct tuple_leaf { T data; };

    template <typename Seq, typename...> struct tuple_impl;

    template <std::size_t... Is, typename... Ts>
    struct tuple_impl<std::index_sequence<Is...>, Ts...> : tuple_leaf<Is, Ts>... {
      constexpr tuple_impl(Ts... args) : tuple_leaf<Is, Ts>{args}... {}
    };

    template<class... Us> tuple_impl(Us...) -> tuple_impl<std::make_index_sequence<sizeof...(Us)>, Us...>;

    template <std::size_t I, typename T> T tuple_element_tag(const tuple_leaf<I, T>&);
  }

  template <typename ... Ts>
  using tuple = details::tuple_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

  template <typename... Ts>
  constexpr tuple<Ts...> make_tuple(Ts... args) {
    return tuple<Ts...>{args...};
  }
}

namespace std {
  template <typename T, std::size_t I> constexpr const T& get(const ::reachability::details::tuple_leaf<I, T>& t) { return t.data; }
  template <typename T, std::size_t I> constexpr T& get(::reachability::details::tuple_leaf<I, T>& t) { return t.data; }
  template <std::size_t I, typename T> constexpr const T& get(const ::reachability::details::tuple_leaf<I, T>& t) { return t.data; }
  template <std::size_t I, typename T> constexpr T& get(::reachability::details::tuple_leaf<I, T>& t) { return t.data; }
  template <std::size_t I, class... Ts> struct tuple_element<I, ::reachability::tuple<Ts...>> {
    using type = decltype(tuple_element_tag<I>(std::declval<::reachability::tuple<Ts...>>()));
  };
  template <class... Ts> struct tuple_size<::reachability::tuple<Ts...>>: std::integral_constant<std::size_t, sizeof...(Ts)> { };
}

namespace reachability {
  // concept written as tempate arg -> template arg -> bool
  // because we cannot pass concepts as template parameters yet (p2841)
  // __cpp_template_parameters >= 202502L
  template <auto count_or_f, auto f=0>
  constexpr auto vec_of = []<class T>{
    if constexpr (!requires { std::tuple_size_v<T>; }) {
      return false;
    } else {
      constexpr std::size_t N = std::tuple_size_v<T>;
      constexpr auto all_of = [&]<auto pred>{
        return []<std::size_t... I>(std::index_sequence<I...>) {
          return (pred.template operator()<typename std::tuple_element_t<I, T>>() && ...);
        }(std::make_index_sequence<N>{});
      };
      if constexpr (std::is_integral_v<decltype(count_or_f)>) {
        return N == count_or_f && all_of.template operator()<f>();
      } else return all_of.template operator()<count_or_f>();
    }
  };

  template <auto... fs>
  constexpr auto tuple_of = []<class T>{
    constexpr std::size_t N = sizeof...(fs);
    if constexpr (!requires { std::tuple_size_v<T>; }) {
      return false;
    } else if constexpr (N != std::tuple_size_v<T>) {
      return false;
    } else {
      constexpr auto fs_tuple = tuple{fs...};
      return [&]<std::size_t... I>(std::index_sequence<I...>) {
        return (std::get<I>(fs_tuple).template operator()<typename std::tuple_element_t<I, T>>() && ...);
      }(std::make_index_sequence<N>{});
    }
  };

  template <class U>
  constexpr auto type_of = []<class T>{
    return std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::remove_cv_t<std::remove_reference_t<U>>>;
  };

  template <typename T, auto f>
  concept Wrap = requires {
    requires f.template operator()<T>();
  };
}
