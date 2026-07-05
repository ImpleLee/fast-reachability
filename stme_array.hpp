#pragma once

#include <array>
#include <cstddef>
#include <concepts>
#include <functional>
#include "utils.hpp"

namespace Shak {
    using namespace reachability;

    template<typename Generator, std::size_t N>
    concept my_epic_generator = []<std::size_t... Indices>(std::index_sequence<Indices...>) {
        /* Can be called with every index. */
        return (
            std::invocable<Generator &, const std::integral_constant<std::size_t, Indices> &> &&
            ...
        );
    }(std::make_index_sequence<N>{});

    // same type multiple element (simd but worse)
    template<typename T, std::size_t N>
    struct stme_array {
        using data_t = std::array<T, N>;
        static_assert(sizeof(data_t) == N * sizeof(T));
        explicit constexpr stme_array() = default;
        public:
        template<my_epic_generator<N> Generator>
        constexpr explicit stme_array(Generator gen) {
            static_for<N>([&](const auto i) {
                data[int(i)] = std::invoke(gen, i);
            });
        }


        constexpr explicit stme_array( std::integral auto value ) noexcept{
            static_for<N>([&](const auto i) {
                data[int(i)] = value;
            });
        }

        template< class U >
        constexpr explicit stme_array( stme_array<U, N> other ) noexcept {
            static_for<N>([&](const auto i) {
                data[int(i)] = other[i];
            });
        }

        constexpr explicit stme_array (data_t other) noexcept {
            data = other;
        }

        constexpr T operator[](int i) const {
            return data[i];
        }

        friend constexpr void assign(stme_array &d, int i, T value) {
            d.data[i] = value;
        }

        constexpr stme_array operator-(stme_array other) const {
            return stme_array{[&](auto i){ return data[i] - other.data[i]; }};
        }

        constexpr stme_array operator+(stme_array other) const {
            return stme_array{[&](auto i){ return data[i] + other.data[i]; }};
        }

        constexpr stme_array operator|(stme_array other) const {
            return stme_array{[&](auto i){ return data[i] | other.data[i]; }};
        }

        constexpr stme_array operator&(stme_array other) const {
            return stme_array{[&](auto i){ return data[i] & other.data[i]; }};
        }

        constexpr stme_array operator^(stme_array other) const {
            return stme_array{[&](auto i){ return data[i] ^ other.data[i]; }};
        }

        constexpr stme_array operator>>(std::integral auto other) const {
            return stme_array{[&](auto i){ return data[i] >> other; }};
        }

        constexpr void operator>>=(stme_array other) {
            static_for<N>([&](auto i) {
                data[i] >>= other.data[i];
            });
        }

        constexpr void operator>>=(std::integral auto other) {
            static_for<N>([&](auto i) {
                data[i] >>= other;
            });
        }

        constexpr void operator<<=(stme_array other) {
            static_for<N>([&](auto i) {
                data[i] <<= other.data[i];
            });
        }

        constexpr void operator<<=(std::integral auto other) {
            static_for<N>([&](auto i) {
                data[i] <<= other;
            });
        }

        constexpr stme_array operator<<(std::integral auto other) const {
            return stme_array{[&](auto i){ return data[i] << other; }};
        }

        constexpr stme_array operator<<(unsigned int other) const {
            return stme_array{[&](auto i){ return data[i] << other; }};
        }

        constexpr stme_array operator==(std::integral auto other) const {
            return stme_array{[&](auto i){ return data[i] == other; }};
        }

        constexpr stme_array operator!=(std::integral auto other) const {
            return stme_array{[&](auto i){ return data[i] != other; }};
        }

        constexpr stme_array operator!=(stme_array other) const {
            return stme_array{[&](auto i){ return data[i] != other.data[i]; }};
        }

        constexpr void operator&=(stme_array other) {
            static_for<N>([&](auto i) {
                data[i] &= other.data[i];
            });
        }

        constexpr void operator|=(stme_array other) {
            static_for<N>([&](auto i) {
                data[i] |= other.data[i];
            });
        }

        constexpr stme_array operator~() const {
            return stme_array{[&](auto i){ return ~data[i]; }};
        }

        constexpr static auto size() {
            return N;
        }

        friend constexpr bool any_of(stme_array self) {
            T any{};
            static_for<N>([&](auto i) {
                any |= self.data[int(i)];
            });

            return !!any;
        }

        [[gnu::always_inline]] friend constexpr bool all_of(stme_array self) {
            T all = -1;
            static_for<N>([&](auto i) {
                all &= self.data[int(i)];
            });

            return !!all;
        }

        data_t data;
    };

};
